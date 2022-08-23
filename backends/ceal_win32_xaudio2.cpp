// Windows API core
#if defined _WIN32 || defined _WIN64

#include "ceal_win32_xaudio2.h"

#include "ceal_loaders.h"
#include "ceal_debug.h"

#include <math.h>
#include <iostream>

#include <chrono>
#include <thread>

// =============================================================================
//					                 Macros
// =============================================================================

// TODO(Urby): Clean this up
#define LEFT_AZIMUTH            3 * X3DAUDIO_PI / 2
#define RIGHT_AZIMUTH           X3DAUDIO_PI / 2
#define FRONT_LEFT_AZIMUTH      7 * X3DAUDIO_PI / 4
#define FRONT_RIGHT_AZIMUTH     X3DAUDIO_PI / 4
#define FRONT_CENTER_AZIMUTH    0.0f
#define LOW_FREQUENCY_AZIMUTH   X3DAUDIO_2PI
#define BACK_LEFT_AZIMUTH       5 * X3DAUDIO_PI / 4
#define BACK_RIGHT_AZIMUTH      3 * X3DAUDIO_PI / 4
#define BACK_CENTER_AZIMUTH     X3DAUDIO_PI

static constexpr float s_ChannelAzimuths[9][8] =
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

#define CEAL_CHOOSE_CHANNEL_AZIMUTHS(__input) const_cast<float*>(s_ChannelAzimuths[__input])
#define CEAL_GROUP_AT(__groupId) g_CealContext->GroupMap.at(*__groupId)
#define CEAL_SOURCE_AT(__sourceId) g_CealContext->SourceMap.at(*__sourceId)

#define CEAL_X3DAUDIO_CALCULATE X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB

namespace Ceal {

    // Debugger
    // Debugger
    // Debugger

	void ContextDebugger::OnProcessingPassEnd() {}
	void ContextDebugger::OnProcessingPassStart() {}

	void ContextDebugger::OnCriticalError(HRESULT error)
{
		std::cout << "On Critical Error: " << error;
	}

    // SourceVoiceCallback
    // SourceVoiceCallback
    // SourceVoiceCallback

    SourceVoiceCallback::SourceVoiceCallback(SourceDetails* sourceDetails)
        : m_SourceDetails(sourceDetails), m_BufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
    {
    }

    SourceVoiceCallback::~SourceVoiceCallback()
    {
        /*CloseHandle(m_BufferEndEvent);*/
    }

    void SourceVoiceCallback::OnBufferEnd(void* pBufferContext)
    {
        SetEvent(m_BufferEndEvent);
    }

    void SourceVoiceCallback::OnStreamEnd()
    {
        printf("OnStreamEnd\n");
        SetEvent(m_BufferEndEvent);
        m_SourceDetails->StreamData.Streaming = false;
    }

    void SourceVoiceCallback::OnBufferStart(void* pBufferContext)
    {
    }

    void SourceVoiceCallback::OnLoopEnd(void* pBufferContext)
    {
        printf("OnLoopEnd\n");
    }

    void SourceVoiceCallback::OnVoiceError(void* pBufferContext, HRESULT error) 
    { 
        printf("OnVoiceError: %d \n", error);
    }

}
namespace Ceal {

// =============================================================================
//					  Functions for internal use only.
// =============================================================================

	namespace Internal {

		/**
		 * @brief Streams audio.
		 * @param sourceDetails
		 * @param bufferMaxCount
		 * @param bufferMaxSize
		 * @return void
		 */
		static void StreamAudio(const SourceDetails* sourceDetails)
		{
            uint32_t bufferMaxCount = sourceDetails->StreamData.BufferMaxCount;
            uint32_t bufferMaxSize = sourceDetails->StreamData.BufferMaxSize;

			// TODO(Urby): Improve this function

			// Allocate enough memory
			BYTE** buffers = new BYTE*[bufferMaxCount];

			for (uint32_t i = 0; i < bufferMaxCount; ++i)
			{
				buffers[i] = new BYTE[bufferMaxSize];
			}

			AudioFile_Wav audioFileInfo;

			GetAudioFileInfo_Wav(sourceDetails->StreamData.Filepath, &audioFileInfo);

			OVERLAPPED overlapped = { 0 };
			overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            overlapped.Offset = audioFileInfo.DataOffset;

			CEAL_ASSERT_XA2(sourceDetails->XSourceVoice->Start(0, 0));

			// size in bytes
			uint32_t cbWaveSize = audioFileInfo.DataSize;

			// Open the file
			HANDLE hFile = CreateFileA(sourceDetails->StreamData.Filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			CEAL_ASSERT(GetLastError() == NO_ERROR); // File not found or something

			// WinApi error checking
			DWORD errorCode = NO_ERROR;

			uint32_t currentDiskReadBuffer = 0;
			uint32_t currentPosition = audioFileInfo.DataOffset; // Start from the data section.
			while (g_CealContext->IsClosing == false && currentPosition < cbWaveSize)
			{
                printf("Bytes so far: %d/%d B\n", currentPosition, cbWaveSize); // TODO(Urby): Remove

				DWORD cbValid = min(bufferMaxSize, cbWaveSize - currentPosition);

				// Read a chunk of data from the disk into the current read buffer.
				DWORD readChunk;
				CEAL_ASSERT(ReadFile(hFile, buffers[currentDiskReadBuffer], bufferMaxSize, &readChunk, &overlapped));

				overlapped.Offset += cbValid;

				// Update the file position to where it will be once the read finishes.
				currentPosition += cbValid;

				// Use the GetOverlappedResult function to wait for the event that signals the read has finished.
				DWORD numberBytesTransferred;
				::GetOverlappedResult(hFile, &overlapped, &numberBytesTransferred, TRUE);
				// Wait for the number of buffers queued on the source voice to be less than the number of read buffers.
				{
					ScopedTimer timer("Streaming");
					XAUDIO2_VOICE_STATE state;
					while (sourceDetails->XSourceVoice->GetState(&state), g_CealContext->IsClosing == false && state.BuffersQueued >= bufferMaxCount - 1)
					{
						HANDLE handles[2] = { sourceDetails->Callback.GetBufferEndEvent(), g_CealContext->ExitEventHandle };

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

				CEAL_ASSERT_XA2(sourceDetails->XSourceVoice->SubmitSourceBuffer(&buf));

				// Set the current read buffer index to the next buffer.

				currentDiskReadBuffer++;
				currentDiskReadBuffer %= bufferMaxCount;
			}

            // Waiting for XAudio2 to finish 
            XAUDIO2_VOICE_STATE state;
            while (sourceDetails->XSourceVoice->GetState(&state), g_CealContext->IsClosing == false && state.BuffersQueued != 0)
            {
            }

            // Closing IO
            CloseHandle(hFile);

			// Freeing up the buffers
			for (uint32_t i = 0; i < bufferMaxCount; ++i)
			{
				delete[] buffers[i];
			}
			delete[] buffers;

            printf("EXIT");

            // TODO(Urby): Figure out how to properly close audio stream thread.
		}

		/**
		 * @brief Setups utils for debugging XAudio2.
		 * @see https://docs.microsoft.com/en-us/windows/win32/xaudio2/debugging-facilities
		 * @return Ceal::Result
		 */
		static Result SetupXAudio2Debug()
		{
			XAUDIO2_DEBUG_CONFIGURATION debugConf{};
			debugConf.LogFunctionName = true;

			// XAUDIO2_LOG_WARNINGS also enables XAUDIO2_LOG_ERRORS
			debugConf.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS |
				XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
			g_CealContext->XInstance->SetDebugConfiguration(&debugConf);

			// Perfomance data
			XAUDIO2_PERFORMANCE_DATA performanceData;
			g_CealContext->XInstance->GetPerformanceData(&performanceData);

			CEAL_CHECK_XA2(g_CealContext->XInstance->RegisterForCallbacks(&g_CealContext->Debugger));

			return Result_Success;
		}

		/**
		 * @brief Calculates channel values according to listener-source attributes.
		 * @return Ceal::Result
		 */
        static Result Calculate3D(SourceDetails* sourceDetails, const X3DAUDIO_LISTENER* listener, float* outputMatrix)
        {
            // Locals
            uint32_t sourceInputChannels = sourceDetails->ChannelCount;
            uint32_t masterInputChannels = g_CealContext->MasterDetails.Details.InputChannels;

            // Emitter 
            X3DAUDIO_EMITTER sourceEmitter;
            {
                sourceEmitter.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone;

                sourceEmitter.OrientFront.x = sourceDetails->Attributes[SourceAttribute_OrientFrontX];
                sourceEmitter.OrientFront.y = sourceDetails->Attributes[SourceAttribute_OrientFrontY];
                sourceEmitter.OrientFront.z = sourceDetails->Attributes[SourceAttribute_OrientFrontZ];
                sourceEmitter.OrientTop.x = sourceDetails->Attributes[SourceAttribute_OrientTopX];
                sourceEmitter.OrientTop.y = sourceDetails->Attributes[SourceAttribute_OrientTopY];
                sourceEmitter.OrientTop.z = sourceDetails->Attributes[SourceAttribute_OrientTopZ];

                sourceEmitter.Position.x = sourceDetails->Attributes[SourceAttribute_PositionX];
                sourceEmitter.Position.y = sourceDetails->Attributes[SourceAttribute_PositionY];
                sourceEmitter.Position.z = sourceDetails->Attributes[SourceAttribute_PositionZ];
                sourceEmitter.Velocity.x = sourceDetails->Attributes[SourceAttribute_VelocityX];
                sourceEmitter.Velocity.y = sourceDetails->Attributes[SourceAttribute_VelocityY];
                sourceEmitter.Velocity.z = sourceDetails->Attributes[SourceAttribute_VelocityZ];

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
                g_CealContext->X3DInstance, listener, &sourceEmitter,
                X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
                X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
                &dspSettings);

            // TODO(Urby): Create struct holding useful infos

            sourceDetails->Attributes[SourceAttribute_Pitch] = dspSettings.DopplerFactor;

            XAUDIO2_FILTER_PARAMETERS filterParameters = {
                LowPassFilter,
                2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient),
                1.0f };
            sourceDetails->XSourceVoice->SetFilterParameters(&filterParameters);

            return Result_Success;
        }

	}

// =============================================================================
//									  Queries
// =============================================================================

    /**
     * @brief Query for source's attribute.
     * @param source ID of the source.
     * @param attribute Source's attribute.
     * @param value Value of the attribute.
     */
    void GetSourceFloat(const Source_T* source, SourceAttribute attribute, float* value)
    {
        auto& sourceDetails = CEAL_SOURCE_AT(source);
        *value = sourceDetails.Attributes[attribute];
    }

// =============================================================================
//									  Functions
// =============================================================================

	/**
	 * @brief Creates Windows-XAudio2 audio context.
     * @param flags Flags of the context.
	 * @return Ceal::Result
	 */
	Result CreateContext(ContextFlags flags)
	{
		CEAL_ASSERT(g_CealContext == nullptr);

		g_CealContext = new CealContext;

		// Create COM
		CEAL_CHECK_XA2(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

		// Create audio engine instance
		CEAL_CHECK_XA2(XAudio2Create(&g_CealContext->XInstance, 0, XAUDIO2_DEFAULT_PROCESSOR));

		// Setup debug and log info for EventViewer by Windows
		CEAL_ASSERT(Internal::SetupXAudio2Debug());

		// Create mastering voice
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateMasteringVoice(&g_CealContext->XMasterVoice, 2));

		// Query details about master voice
		g_CealContext->XMasterVoice->GetVoiceDetails(&g_CealContext->MasterDetails.Details);
		g_CealContext->XMasterVoice->GetChannelMask(&g_CealContext->MasterDetails.ChannelMask);

		// Initialize 3D Audio
		CEAL_CHECK_XA2(X3DAudioInitialize(g_CealContext->MasterDetails.ChannelMask, X3DAUDIO_SPEED_OF_SOUND, g_CealContext->X3DInstance));

        // Initiliaze XListener to default values
        memset(g_CealContext->ListenerAttributes, 0, sizeof(g_CealContext->ListenerAttributes));
        g_CealContext->ListenerAttributes[ListenerAttribute_OrientFrontX] = 1.0f;
        g_CealContext->ListenerAttributes[ListenerAttribute_OrientTopZ] = 1.0f;
        //g_CealContext->XListener.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone; // TODO(Urby): Learn how cones work

        // Zero is invalid
        g_CealContext->GlobalIncrementId = 1;
        g_CealContext->Flags = flags;
        g_CealContext->IsClosing = false;

		return Result_Success;
	}

	/**
	 * @brief Destroys Windows-XAudio2 context.
	 * @return Ceal::Result
	 */
	Result DestroyContext()
	{
		// Notify thread that the context is being destroyed
		g_CealContext->IsClosing = true;
		SetEvent(g_CealContext->ExitEventHandle);

		// Release source voices
		for (auto& [id, sourceVoice] : g_CealContext->SourceMap)
		{
			if (sourceVoice.StreamData.Thread.joinable())
			{
				sourceVoice.StreamData.Thread.join();
			}

			CEAL_CHECK_XA2(sourceVoice.XSourceVoice->Stop());
			sourceVoice.XSourceVoice->DestroyVoice();
		}

		// Releasing audio engine
		g_CealContext->XMasterVoice->DestroyVoice();
		g_CealContext->XInstance->Release();

        // Lastly release buffers
        for (const auto& [id, buffer] : g_CealContext->XBufferMap)
        {
            delete buffer.pAudioData;
        }

		delete g_CealContext;

		return Result_Success;
	}

    /**
     * @brief Sets user specific pointer to data.
     * @param userPointer
     */
    void SetUserPointer(void* userPointer)
    {
        g_CealContext->UserPointer = userPointer;
    }

    /**
     * @brief Gets user specific pointer to data.
     * @param userPointer
     */
    void GetUserPointer(void* userPointer)
    {
        userPointer = g_CealContext->UserPointer;
    }

	/**
	 * @brief Creates group
	 * @param group ID of the group.
	 * @return Ceal::Result
	 */
	Result CreateGroup(Group_T* group)
	{
		group->ID = ++g_CealContext->GlobalIncrementId;
		auto& submixGroup = g_CealContext->GroupMap[*group];
		//CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSubmixVoice(&submixGroup.XSubmixVoice, g_CealContext->XMasterVoiceDetails.InputChannels, 44100, 0, 0, 0, 0));
		return Result_Success;
	}

	/**
	 * @brief Creates source and attach it into the specified group.
	 * @param source ID of the source.
	 * @param audioFile Struct populated with audio file information.
	 * @param group ID of the group.
	 * @return Ceal::Result
	 */
	Result CreateSource(Source_T* source, const AudioFile_Wav* audioFile, const Group_T* group/* = nullptr */)
	{
		// Create Source Voice
		source->ID = ++g_CealContext->GlobalIncrementId;
		auto& sourceDetails = g_CealContext->SourceMap[*source];

		// Populating WAVEFORMATEX structure
		WAVEFORMATEX wfx = { 0 };
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = audioFile->NumChannels;
		wfx.nSamplesPerSec = audioFile->SampleRate;
		wfx.wBitsPerSample = audioFile->BitsPerSample;
		wfx.cbSize = audioFile->ExtraParamSize;
		wfx.nBlockAlign = audioFile->BlockAlign;
		wfx.nAvgBytesPerSec = audioFile->ByteRate;

        // Default values for emitter attributes
        sourceDetails.ChannelCount = audioFile->NumChannels; // TODO(Urby): Figure out how to implement multichannel
        sourceDetails.CurveDistanceScaler = 1.0f;
        sourceDetails.ChannelRadius = 1.0f;

        memset(sourceDetails.Attributes, 0, sizeof(sourceDetails.Attributes));
        g_CealContext->ListenerAttributes[ListenerAttribute_OrientFrontX] = 1.0f;
        g_CealContext->ListenerAttributes[ListenerAttribute_OrientTopZ] = 1.0f;
        sourceDetails.Attributes[SourceAttribute_Volume] = 1.0f;
        sourceDetails.Attributes[SourceAttribute_Pitch] = 1.0f;
        //sourceDetails.Emitter3D.InnerRadius = 2.0f;
        //sourceDetails.Emitter3D.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
        //sourceDetails.Emitter3D.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone;

		// Creating XAudio2 source voice
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSourceVoice(&sourceDetails.XSourceVoice, &wfx, NULL, XAUDIO2_DEFAULT_FREQ_RATIO, &sourceDetails.Callback, NULL));

		// Assigning source voice to group 
		if (group)
		{
			auto& sourceVoice = CEAL_SOURCE_AT(source);
			sourceVoice.Group = *group;

			// Adding source's handle to group
			auto& sourceIDs = CEAL_GROUP_AT(group).SourceIDs;
			sourceIDs.push_back(*source);
		}

		return Result_Success;
	}

	/**
	 * @brief Creates buffer from file.
	 * @param buffer ID of the buffer.
	 * @param audioFile Struct populated with audio file information.
	 * @return Ceal::Result
	 */
	Result CreateBuffer(Buffer_T* buffer, const AudioFile_Wav* audioFile)
	{
		buffer->ID = ++g_CealContext->GlobalIncrementId;

		// Populating XAUDIO2_BUFFER structure
		auto& bufferDetails = g_CealContext->XBufferMap[*buffer];
		bufferDetails.AudioBytes = audioFile->DataSize;	// Size of the audio buffer in bytes
		bufferDetails.pAudioData = audioFile->Data;		// Buffer containing audio data
		bufferDetails.Flags = XAUDIO2_END_OF_STREAM;    // Tell the source voice not to expect any data after this buffer

		return Result_Success;
	}

	/**
	 * @brief Submits buffer to specified source.
	 * @param source Pointer to source struct.
	 * @param buffer Pointer to buffer struct.
	 * @return Ceal::Result
	 */
	Result SubmitBuffer(const Source_T* source, const Buffer_T* bufferId)
	{
		// Query for buffer and source
		auto& buffer = g_CealContext->XBufferMap.at(*bufferId);
		auto& sourceDetails = CEAL_SOURCE_AT(source);

		// Submitting buffer
		CEAL_CHECK_XA2(sourceDetails.XSourceVoice->SubmitSourceBuffer(&buffer));

		return Result_Success;
	}

	/**
	 * @brief Plays submitted buffer on specified source.
	 * @param source ID of the source.
	 * @return Ceal::Result
	 */
	Result Play(const Source_T* source)
	{
		auto& sourceDetails = CEAL_SOURCE_AT(source);
		// Playing audio
		CEAL_CHECK_XA2(sourceDetails.XSourceVoice->Start(0, XAUDIO2_COMMIT_NOW));

		return Result_Success;
	}

	/**
	 * @brief Plays submitted buffers on specified group of sources.
	 * @param group ID of the group.
	 * @return Ceal::Result
	 */
	Result Play(const Group_T* group)
	{
		auto& groupDetails = CEAL_GROUP_AT(group);

		// Playing audio
		for (auto& source : groupDetails.SourceIDs)
		{
			auto& sourceDetails = CEAL_SOURCE_AT(&source);

			CEAL_CHECK_XA2(sourceDetails.XSourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
		}
		return Result_Success;
	}

	/**
	 * @brief Updates source attributes.
	 * @param source ID of the source.
	 * @param property Property to be to changed.
	 * @param value New value of the property.
	 */
	void SetSourceFloat(const Source_T* source, SourceAttribute attribute, float value)
	{
		auto& sourceDetails = CEAL_SOURCE_AT(source);

        sourceDetails.Attributes[attribute] = value; 
	}

    /**
    * @brief Updates listeners attribute.
    * @param attribute Attribute to be changed.
    * @param value New value of the attribute.
    */
    void SetListenerFloat(ListenerAttribute attribute, float value)
    {
        g_CealContext->ListenerAttributes[attribute] = value;
    }

	/**
	 * @brief Sets context flags.
	 * @param flags Flags to be changed.
	 */
	void SetContextFlags(ContextFlags flags)
	{
		g_CealContext->Flags |= flags;
	}

    /**
     * @brief Unsets context flags.
     * @param flags Flags to be changed.
     * @see => ContextFlags_
     */
    void UnsetContextFlags(ContextFlags flags)
    {
        g_CealContext->Flags &= ~flags;
    }

	/**
	 * @brief Updates Context. Call this function every frame.
	 * Note that source's pitch will be overridden when spatial sound is enabled.
	 * @return Ceal::Result
	 */
	Result Update()
	{
        // Listener
        static X3DAUDIO_LISTENER s_Listener;
        if (g_CealContext->Flags & ContextFlags_Enable3D)
        {
            /**
             * To simplify populating X3DAUDIO_LISTENER struct,
             * We can memcpy values directly from Context's ListenerAttributes member.
             * However to do that firstly X3DAUDIO_LISTENER needs to be packed which fortunately is,
             * and secondary ListenerAttribute_ needs to match X3DAUDIO_LISTENER struct layout.
             */
            memcpy(&s_Listener, g_CealContext->ListenerAttributes, sizeof(g_CealContext->ListenerAttributes));

            // TODO(Urby): Add option for client to change X3DAUDIO_CONE attribute 
            s_Listener.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone;
        }

        // Iterates through avaiable sources and update their content.
        for (auto& [source, sourceDetails] : g_CealContext->SourceMap)
        {
            // No need for to be heap allocated, just allocate maximum possible.
            float outputMatrix[CEAL_MAX_CHANNELS * CEAL_MAX_CHANNELS]{ 1.0f };
            if (g_CealContext->Flags & ContextFlags_Enable3D) 
            {
                Internal::Calculate3D(&sourceDetails, &s_Listener, outputMatrix);
            }

            CEAL_CHECK_XA2(sourceDetails.XSourceVoice->SetVolume(sourceDetails.Attributes[SourceAttribute_Volume]));
            CEAL_CHECK_XA2(sourceDetails.XSourceVoice->SetFrequencyRatio(sourceDetails.Attributes[SourceAttribute_Pitch]));
            CEAL_CHECK_XA2(sourceDetails.XSourceVoice->SetOutputMatrix(NULL, sourceDetails.ChannelCount, g_CealContext->MasterDetails.Details.InputChannels, outputMatrix));
        }

        return Result_Success;
	}

// =============================================================================
//                                  Streaming
// =============================================================================

    /**
     * @brief Creates asynchronous audio stream.
     * @param source ID of the source.
     * @param filepath Path to the audio file.
     * @return Ceal::Result
     */
	Result CreateStream(const Source_T* source, const char* filepath)
	{
        CEAL_ASSERT(filepath); // Invalid filepath!

		auto& sourceDetails = CEAL_SOURCE_AT(source);
        sourceDetails.StreamData.Filepath = filepath;
        sourceDetails.StreamData.BufferMaxCount = 3;
        sourceDetails.StreamData.BufferMaxSize = 65536;

		return Result_Success;
	}

	/**
	 * @brief Plays audio stream. Do not submit any buffers while streaming.
	 * @param source ID of the source.
	 * @return Ceal::Result
	 */
	Result PlayStream(const Source_T* source)
	{
		auto& sourceDetails = CEAL_SOURCE_AT(source);
		CEAL_PASS(sourceDetails.StreamData.Streaming); // Stream is already playing!

		// Starting the thread 
        sourceDetails.StreamData.Thread = std::thread([&sourceDetails]()
        {
            Internal::StreamAudio(&sourceDetails);
        });
		sourceDetails.StreamData.Streaming = true;
		return Result_Success;
	}

} // namespace Ceal

#endif // defined _WIN32 || defined _WIN64
