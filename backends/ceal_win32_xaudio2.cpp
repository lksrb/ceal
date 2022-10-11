// =============================================================================
//					   XAudio2-Windows Backend Implementation
// =============================================================================
#ifdef _WIN32

#include "ceal_win32_xaudio2.h"

#include "ceal.h"
#include "ceal_common.h"
#include "ceal_debug.h"
#include "ceal_internal.h"
#include "ceal_loaders.h"

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib")

#include <math.h>

#include <unordered_map>
#include <chrono>
#include <new>
#include <thread>
#include <iostream>

// =============================================================================
//					                 Macros
// =============================================================================

// MSVC only
#ifdef _MSC_VER
#   define CEAL_XA2_CHECK(x) { HRESULT hr = (x); if(hr != S_OK) { return CealResult_Failed;  }  }
#   define CEAL_XA2_ASSERT(x) { HRESULT hr = (x); if(hr != S_OK) { __debugbreak();  }  }
#endif

#define LEFT_AZIMUTH            3 * X3DAUDIO_PI / 2
#define RIGHT_AZIMUTH           X3DAUDIO_PI / 2
#define FRONT_LEFT_AZIMUTH      7 * X3DAUDIO_PI / 4
#define FRONT_RIGHT_AZIMUTH     X3DAUDIO_PI / 4
#define FRONT_CENTER_AZIMUTH    0.0f
#define LOW_FREQUENCY_AZIMUTH   X3DAUDIO_2PI
#define BACK_LEFT_AZIMUTH       5 * X3DAUDIO_PI / 4
#define BACK_RIGHT_AZIMUTH      3 * X3DAUDIO_PI / 4
#define BACK_CENTER_AZIMUTH     X3DAUDIO_PI

static const float s_ChannelAzimuths[9][8] =
{
    /* 0 */   { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* 1 */   { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* 2 */   { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    /* 2.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, LOW_FREQUENCY_AZIMUTH, 0.f, 0.f, 0.f, 0.f, 0.f },
    /* 4.0 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f, 0.f, 0.f },
    /* 4.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f, 0.f },
    /* 5.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f },
    /* 6.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, BACK_CENTER_AZIMUTH, 0.f },
    /* 7.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, LEFT_AZIMUTH, RIGHT_AZIMUTH }
};

#undef LEFT_AZIMUTH           
#undef RIGHT_AZIMUTH          
#undef FRONT_LEFT_AZIMUTH     
#undef FRONT_RIGHT_AZIMUTH    
#undef FRONT_CENTER_AZIMUTH   
#undef LOW_FREQUENCY_AZIMUTH  
#undef BACK_LEFT_AZIMUTH      
#undef BACK_RIGHT_AZIMUTH     
#undef BACK_CENTER_AZIMUTH    

#define CEAL_MAX_CHANNELS 8

// Helpers
#define CEAL_CHOOSE_CHANNEL_AZIMUTHS(__input) const_cast<float*>(s_ChannelAzimuths[__input])
#define CEAL_SOURCE_AT(__source) s_XAudio2Win32_Backend->XSourceMap.at(__source)
#define CEAL_BUFFER_AT(__buffer) s_XAudio2Win32_Backend->XBufferMap.at(__buffer)

// Global context
extern CealGlobalContext* g_GlobalContext;

// =============================================================================
//					              Callbacks
// =============================================================================

class XAudio2DebuggerCallback : public IXAudio2EngineCallback
{
    void OnProcessingPassEnd()
    {

    }
    void OnProcessingPassStart()
    {

    }
    void OnCriticalError(HRESULT error)
    {
        printf("Error %d", error);
    }
};

class SourceVoiceCallback : public IXAudio2VoiceCallback
{
public:
    SourceVoiceCallback() : m_Source(nullptr) {}

    void SetSource(CealSource source)
    {
        m_Source = source;
        m_Source->StreamData.BufferEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_Source->StreamData.StreamCallback = this;
    }

    ~SourceVoiceCallback()
    {
    }

protected:
    void OnBufferStart(void* pBufferContext)
    {

    }
    void OnBufferEnd(void* pBufferContext)
    {
        if (m_Source->StreamData.IsStreaming)
            SetEvent(m_Source->StreamData.BufferEndEvent);
        else
            m_Source->Occupied = false;
    }

    void OnVoiceProcessingPassStart(UINT32 bytesRequired) {}
    void OnVoiceProcessingPassEnd() {};

    void OnStreamEnd()
    {
        printf("OnStreamEnd\n");
        SetEvent(m_Source->StreamData.BufferEndEvent);
        m_Source->StreamData.IsStreaming = false;
    }
    void OnLoopEnd(void* pBufferContext)
    {
        printf("OnLoopEnd\n");
    }

    void OnVoiceError(void* pBufferContext, HRESULT error)
    {
        printf("Critical Error: %d", error);
    }
private:
    CealSource m_Source;
};

struct XAudio2_Win32_Backend
{
    // XAudio2 objects
    IXAudio2*                   XInstance;                              // Handle to XAudio2's instance
    IXAudio2MasteringVoice*     XMasterVoice;                           // Handle to XAudio2's mastering voice
    XAUDIO2_VOICE_DETAILS       Details;                                // Audio device Details
    DWORD                       ChannelMask;                            // Audio device channel mask
    X3DAUDIO_HANDLE             X3DInstance;                            // Handle to X3DAudio
    XAudio2DebuggerCallback     Debugger;                               // XAudio2 debugging

    //std::vector<SourceVoiceCallback> Callbacks;                       // TODO(Urby): Figure out how to setup callbacks for audio streaming.

    std::unordered_map<CealBuffer, XAUDIO2_BUFFER> XBufferMap;          // Buffers
    std::unordered_map<CealSource, IXAudio2SourceVoice*> XSourceMap;    // Sources

    // Multithreading // Should be in separate context
    bool IsClosing;
    HANDLE ExitEventHandle;
};

static XAudio2_Win32_Backend* s_XAudio2Win32_Backend = nullptr; // TODO: Separate win32 and xaudio2 context

// =============================================================================
//					  Functions for internal use only.
// =============================================================================

/**
 * @brief Setups utils for debugging XAudio2.
 * @see https://docs.microsoft.com/en-us/windows/win32/xaudio2/debugging-facilities
 * @return Ceal::Result
 */
static CealResult ceal_internal_win32_xaudio2_debug_setup()
{
    XAUDIO2_DEBUG_CONFIGURATION debugConf{};
    debugConf.LogFunctionName = true;

    // XAUDIO2_LOG_WARNINGS also enables XAUDIO2_LOG_ERRORS
    debugConf.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS |
        XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
    s_XAudio2Win32_Backend->XInstance->SetDebugConfiguration(&debugConf);

    // Perfomance data
    XAUDIO2_PERFORMANCE_DATA performanceData;
    s_XAudio2Win32_Backend->XInstance->GetPerformanceData(&performanceData);

    CEAL_XA2_CHECK(s_XAudio2Win32_Backend->XInstance->RegisterForCallbacks(&s_XAudio2Win32_Backend->Debugger));

    return CealResult_Success;
}

/**
 * @brief Calculates channel values according to listener-source attributes.
 * @return CealResult
 */
static CealResult ceal_internal_win32_xaudio2_calculate3d(CealSource source, const X3DAUDIO_LISTENER* listener, float* outputMatrix)
{
    // Locals
    uint32_t sourceInputChannels = source->ChannelCount;
    uint32_t masterInputChannels = s_XAudio2Win32_Backend->Details.InputChannels;

    // Emitter 
    X3DAUDIO_EMITTER sourceEmitter;
    {
        sourceEmitter.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone;

        sourceEmitter.Position.x = source->Attributes[CealSourceAttribute_PositionX];
        sourceEmitter.Position.y = source->Attributes[CealSourceAttribute_PositionY];
        sourceEmitter.Position.z = source->Attributes[CealSourceAttribute_PositionZ];
        sourceEmitter.Velocity.x = source->Attributes[CealSourceAttribute_VelocityX];
        sourceEmitter.Velocity.y = source->Attributes[CealSourceAttribute_VelocityY];
        sourceEmitter.Velocity.z = source->Attributes[CealSourceAttribute_VelocityZ];
        sourceEmitter.OrientTop.x = source->Attributes[CealSourceAttribute_OrientTopX];
        sourceEmitter.OrientTop.y = source->Attributes[CealSourceAttribute_OrientTopY];
        sourceEmitter.OrientTop.z = source->Attributes[CealSourceAttribute_OrientTopZ];
        sourceEmitter.OrientFront.x = source->Attributes[CealSourceAttribute_OrientFrontX];
        sourceEmitter.OrientFront.y = source->Attributes[CealSourceAttribute_OrientFrontY];
        sourceEmitter.OrientFront.z = source->Attributes[CealSourceAttribute_OrientFrontZ];

        sourceEmitter.InnerRadius = 1.0f;
        sourceEmitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
        sourceEmitter.ChannelCount = sourceInputChannels;
        sourceEmitter.ChannelRadius = 1.0f;
        sourceEmitter.pChannelAzimuths = CEAL_CHOOSE_CHANNEL_AZIMUTHS(sourceInputChannels);

        sourceEmitter.pVolumeCurve = NULL;
        sourceEmitter.pLFECurve = NULL;
        sourceEmitter.pLPFDirectCurve = NULL;
        sourceEmitter.pLPFReverbCurve = NULL;
        sourceEmitter.pReverbCurve = NULL;
        sourceEmitter.CurveDistanceScaler = 1.0f;
        sourceEmitter.DopplerScaler = 0.0f;
    }

    // DSP Settings
    X3DAUDIO_DSP_SETTINGS dspSettings{};
    dspSettings.SrcChannelCount = sourceInputChannels;
    dspSettings.DstChannelCount = masterInputChannels;
    dspSettings.pMatrixCoefficients = outputMatrix;

    // Calculating
    X3DAudioCalculate(
        s_XAudio2Win32_Backend->X3DInstance, listener, &sourceEmitter,
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
        X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
        &dspSettings);

    source->Attributes[CealSourceAttribute_Pitch] = dspSettings.DopplerFactor;
    source->Attributes[CealSourceAttribute_Filter_Frequency] = dspSettings.LPFDirectCoefficient ? 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient) : 1.0f;
    source->Attributes[CealSourceAttribute_Filter_OneOverQ] = 1.0f;

    return CealResult_Success;
}


// =============================================================================
//									  Functions
// =============================================================================

/**
 * @brief Creates Windows-XAudio2 audio context.
 */
CealResult ceal_backend_win32_xaudio2_init()
{
    CEAL_ASSERT(g_GlobalContext);                       // Call CEAL::CreateContext() first.
    CEAL_ASSERT(s_XAudio2Win32_Backend == nullptr);     // Only one instance of specific backend is allowed.

    s_XAudio2Win32_Backend = CEAL_NEW(XAudio2_Win32_Backend);
    // Settings audio backend for global context
    g_GlobalContext->Backend = s_XAudio2Win32_Backend;

    // Create COM
    CEAL_XA2_CHECK(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    // Create audio engine instance
    CEAL_XA2_CHECK(XAudio2Create(&s_XAudio2Win32_Backend->XInstance, 0, XAUDIO2_DEFAULT_PROCESSOR));

    // Setup debug and log info for EventViewer by Windows
    CEAL_ASSERT(ceal_internal_win32_xaudio2_debug_setup());

    // Create mastering voice
    CEAL_XA2_CHECK(s_XAudio2Win32_Backend->XInstance->CreateMasteringVoice(&s_XAudio2Win32_Backend->XMasterVoice, 2));

    // Query details about master voice
    s_XAudio2Win32_Backend->XMasterVoice->GetVoiceDetails(&s_XAudio2Win32_Backend->Details);
    s_XAudio2Win32_Backend->XMasterVoice->GetChannelMask(&s_XAudio2Win32_Backend->ChannelMask);

    // Initialize 3D Audio
    CEAL_XA2_CHECK(X3DAudioInitialize(s_XAudio2Win32_Backend->ChannelMask, X3DAUDIO_SPEED_OF_SOUND, s_XAudio2Win32_Backend->X3DInstance));

    // Moved listener's attributes to global context
    // Moved global increment id to global context
    // Moved context flags to global context

    // TODO(Urby): Figure out if each backend needs separate IsClosing attribute.
    s_XAudio2Win32_Backend->IsClosing = false;

    return CealResult_Success;
}

/**
 * @brief Destroys Windows-XAudio2 backend.
 */
CealResult ceal_backend_win32_xaudio2_shutdown()
{
    // Reset global context's pointer to backend.
    g_GlobalContext->Backend = nullptr;

    // Notify thread that the context is being destroyed
    s_XAudio2Win32_Backend->IsClosing = true;
    SetEvent(s_XAudio2Win32_Backend->ExitEventHandle);

    // Release source voices
    for (auto& [source, sourceVoice] : s_XAudio2Win32_Backend->XSourceMap)
    {
        // Wait for thread to clean up
        while (source->StreamData.IsStreaming)
        {
        }

        SourceVoiceCallback* callback = (SourceVoiceCallback*)source->StreamData.StreamCallback;
        CEAL_DELETE(SourceVoiceCallback, callback);

        CEAL_XA2_CHECK(sourceVoice->Stop());
        sourceVoice->DestroyVoice();
    }

    // Releasing audio engine
    s_XAudio2Win32_Backend->XMasterVoice->DestroyVoice();
    s_XAudio2Win32_Backend->XInstance->Release();

    // Manually releasing memory from members.
    CEAL_DELETE(XAudio2_Win32_Backend, s_XAudio2Win32_Backend);

    // Freed by CEAL::FreeMemoryPool
    //delete s_XAudio2Win32_Backend;

    return CealResult_Success; // TODO: Fix memory access violation in streaming
}

// =============================================================================
//								Defined Functions
// =============================================================================

/**
 * @brief Plays submitted buffer on specified source.
 * @param source ID of the source.
 * @return Ceal::Result
 */
CealResult ceal_internal_source_play(CealSource source)
{
    auto& xsource = CEAL_SOURCE_AT(source);
    // Playing audio
    CEAL_ASSERT(xsource->Start(0, XAUDIO2_COMMIT_NOW) == S_OK);

    return CealResult_Success;
}

/**
 * @brief Submits buffer to specified source.
 * @param source Pointer to source struct.
 * @param buffer Pointer to buffer struct.
 * @return Ceal::Result
 */
CealResult ceal_internal_buffer_submit(CealSource source, CealBuffer buffer)
{
    // Query for buffer and source
    auto& xbuffer = CEAL_BUFFER_AT(buffer);
    auto& xsource = CEAL_SOURCE_AT(source);

    // Submitting buffer
    CEAL_ASSERT(xsource->SubmitSourceBuffer(&xbuffer) == S_OK);

    return CealResult_Success;
}

CealResult ceal_internal_source_create(CealSource source, const CealAudioFile_Wav* audioFile)
{
    // Populating WAVEFORMATEX structure
    WAVEFORMATEX wfx = { 0 };
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = audioFile->NumChannels;
    wfx.nSamplesPerSec = audioFile->SampleRate;
    wfx.wBitsPerSample = audioFile->BitsPerSample;
    wfx.cbSize = audioFile->ExtraParamSize;
    wfx.nBlockAlign = audioFile->BlockAlign;
    wfx.nAvgBytesPerSec = audioFile->ByteRate;

    auto& xsource = s_XAudio2Win32_Backend->XSourceMap[source];

    SourceVoiceCallback* callback = CEAL_NEW(SourceVoiceCallback);
    callback->SetSource(source);

    // Creating XAudio2 source voice
    CEAL_XA2_CHECK(s_XAudio2Win32_Backend->XInstance->CreateSourceVoice(&xsource, &wfx, NULL,
        XAUDIO2_DEFAULT_FREQ_RATIO, callback, NULL, NULL));

    return CealResult_Success;
}

/**
 * @brief Creates buffer from file.
 * @param buffer ID of the buffer.
 * @param audioFile Struct populated with audio file information.
 * @return Ceal::Result
 */
CealResult ceal_internal_buffer_create(CealBuffer buffer, const CealAudioFile_Wav* audioFile)
{
    // Populating XAUDIO2_BUFFER structure
    auto& xbuffer = s_XAudio2Win32_Backend->XBufferMap[buffer];

    xbuffer.AudioBytes = audioFile->DataSize;	// Size of the audio buffer in bytes
    xbuffer.pAudioData = audioFile->Data;		// Buffer containing audio data
    xbuffer.Flags = XAUDIO2_END_OF_STREAM;      // Tell the source voice not to expect any data after this buffer

    return CealResult_Success;
}

CealResult ceal_internal_context_update()
{
    // Listener
    static X3DAUDIO_LISTENER s_Listener;
    if (g_GlobalContext->Flags & CealContextFlags_Enable3D)
    {
        /**
         * To simplify populating X3DAUDIO_LISTENER struct,
         * We can memcpy values directly from Context's ListenerAttributes member.
         * However to do that firstly X3DAUDIO_LISTENER needs to be packed which fortunately is,
         * and secondary ListenerAttribute_ needs to match X3DAUDIO_LISTENER struct layout.
         */
         //memcpy(&s_Listener, g_GlobalContext->ListenerAttributes, sizeof(g_GlobalContext->ListenerAttributes));

        s_Listener.OrientFront.x = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientFrontX];
        s_Listener.OrientFront.y = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientFrontY];
        s_Listener.OrientFront.z = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientFrontZ];

        s_Listener.OrientTop.x = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientTopX];
        s_Listener.OrientTop.y = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientTopY];
        s_Listener.OrientTop.z = g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientTopZ];

        s_Listener.Position.x = g_GlobalContext->ListenerAttributes[CealListenerAttribute_PositionX];
        s_Listener.Position.y = g_GlobalContext->ListenerAttributes[CealListenerAttribute_PositionY];
        s_Listener.Position.z = g_GlobalContext->ListenerAttributes[CealListenerAttribute_PositionZ];

        s_Listener.Velocity.x = g_GlobalContext->ListenerAttributes[CealListenerAttribute_VelocityX];
        s_Listener.Velocity.y = g_GlobalContext->ListenerAttributes[CealListenerAttribute_VelocityY];
        s_Listener.Velocity.z = g_GlobalContext->ListenerAttributes[CealListenerAttribute_VelocityZ];

        // Listener's cone
        X3DAUDIO_CONE audioCone;
        audioCone.InnerAngle = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerAngle];
        audioCone.OuterAngle = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterAngle];
        audioCone.InnerVolume = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerVolume];
        audioCone.OuterVolume = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterVolume];
        audioCone.InnerLPF = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerLPF];
        audioCone.OuterLPF = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterLPF];
        audioCone.InnerReverb = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerReverb];
        audioCone.OuterReverb = g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterReverb];
        s_Listener.pCone = &audioCone;
    }

    // Iterates through avaiable sources and update their content.
    for (auto& [source, xsource] : s_XAudio2Win32_Backend->XSourceMap)
    {
        // No need for to be heap allocated, just allocate maximum possible.
        float outputMatrix[CEAL_MAX_CHANNELS * CEAL_MAX_CHANNELS]{ 1.0f };
        if (g_GlobalContext->Flags & CealContextFlags_Enable3D)
        {
            ceal_internal_win32_xaudio2_calculate3d(source, &s_Listener, outputMatrix);

            // Filter settings
            XAUDIO2_FILTER_PARAMETERS filterParameters;
            filterParameters.Type = LowPassFilter;
            filterParameters.Frequency = source->Attributes[CealSourceAttribute_Filter_Frequency];
            filterParameters.OneOverQ = source->Attributes[CealSourceAttribute_Filter_OneOverQ];

            // TODO(Urby): Figure out what filtering is.
            // Set parameter to specific source
            // CEAL_CHECK_XA2(xsource->SetFilterParameters(&filterParameters));
        }

        CEAL_XA2_CHECK(xsource->SetVolume(source->Attributes[CealSourceAttribute_Volume]));
        CEAL_XA2_CHECK(xsource->SetFrequencyRatio(source->Attributes[CealSourceAttribute_Pitch]));
        CEAL_XA2_CHECK(xsource->SetOutputMatrix(NULL, source->ChannelCount, s_XAudio2Win32_Backend->Details.InputChannels, outputMatrix));
    }
    return CealResult_Success;
}

/**
 * @brief Streams audio function.  
 * @param source Streaming source
 */
void ceal_internal_stream_function(CealSource source) // TODO: Simplify
{
    CealAudioFile_Wav       audioFileInfo;                                          // Struct holding info about audio file.
    uint32_t                bufferMaxCount = source->StreamData.BufferMaxCount;     // Maximum count of buffers streaming audio.
    uint32_t                bufferMaxSize = source->StreamData.BufferMaxSize;       // Maximum size of each buffer.
    IXAudio2SourceVoice*    xsource = CEAL_SOURCE_AT(source);                       // Locates IXAudio2SourceVoice* struct.
    DWORD                   errorCode = NO_ERROR;                                   // WinApi error checking
    uint32_t                cbWaveSize = 0;                                         // Size in bytes
    BYTE** buffers =        (BYTE**)(source->StreamData.Buffers);

    ceal_audio_file_wav_info(source->StreamData.Filepath, &audioFileInfo);      // Retrieves info about audio file.
    cbWaveSize = audioFileInfo.DataSize;

    OVERLAPPED overlapped{};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlapped.Offset = audioFileInfo.DataOffset;

    // Starting source
    CEAL_XA2_ASSERT(xsource->Start(0, 0));

    // Open the file
    HANDLE hFile = CreateFileA(source->StreamData.Filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL); // Opens file
    CEAL_ASSERT(GetLastError() == NO_ERROR); // File not found or something

    uint32_t currentDiskReadBuffer = 0;
    uint32_t currentPosition = audioFileInfo.DataOffset; // Start from the data section.
    while (s_XAudio2Win32_Backend->IsClosing == false && currentPosition < cbWaveSize)
    {
        printf("Bytes so far: %d/%d B\n", currentPosition, cbWaveSize); // TODO: Remove

        DWORD cbValid = min(bufferMaxSize, cbWaveSize - currentPosition);

        // Read a chunk of data from the disk into the current read buffer.

        DWORD readChunk;
        CEAL_ASSERT(ReadFile(hFile, buffers[currentDiskReadBuffer], bufferMaxSize, &readChunk, &overlapped));

        overlapped.Offset += cbValid;
        currentPosition += cbValid; // Update the file position to where it will be once the read finishes.

        // Use the GetOverlappedResult function to wait for the event that signals the read has finished.
        DWORD numberBytesTransferred;
        ::GetOverlappedResult(hFile, &overlapped, &numberBytesTransferred, TRUE);
        // Wait for the number of buffers queued on the source voice to be less than the number of read buffers.
        {
            CEAL_MEASURE_SCOPE("Streaming");
            XAUDIO2_VOICE_STATE state;

            if (s_XAudio2Win32_Backend->IsClosing)
                break;

            while (xsource->GetState(&state), state.BuffersQueued >= bufferMaxCount - 1)
            {
                HANDLE handles[2] = { source->StreamData.BufferEndEvent, s_XAudio2Win32_Backend->ExitEventHandle };
                // TODO(Urby): Re-implement streaming
                WaitForMultipleObjects(2, handles, false, INFINITE);
            }

        }
        // Submit the current read buffer to the source voice using the SubmitSourceBuffer function.
        XAUDIO2_BUFFER buf = { 0 };
        buf.AudioBytes = cbValid;
        buf.pAudioData = buffers[currentDiskReadBuffer];
        if (currentPosition >= cbWaveSize)
        {
            buf.Flags = XAUDIO2_END_OF_STREAM;
        }

        if (s_XAudio2Win32_Backend->IsClosing)
            break;

        CEAL_XA2_ASSERT(xsource->SubmitSourceBuffer(&buf));

        // Set the current read buffer index to the next buffer.
        currentDiskReadBuffer++;
        currentDiskReadBuffer %= bufferMaxCount;
    }

    // Waiting for XAudio2 to finish 
    XAUDIO2_VOICE_STATE state;

    if (s_XAudio2Win32_Backend->IsClosing == false)
    {
        while (xsource->GetState(&state), state.BuffersQueued != 0)
        {
        }
    }
    // TODO: Remake
    // Closing IO
    CloseHandle(hFile);

    xsource->Stop();

    printf("STREAM EXIT");
}

#endif // defined _WIN32 
