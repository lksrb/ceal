#pragma once

#include "ceal.h"

#include <vector>
#include <unordered_map>
#include <thread>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib") 

#define CEAL_CHECK_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { return Ceal::Result_Failed;  }  }
#define CEAL_ASSERT_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { __debugbreak();  }  }

namespace Ceal {

// =============================================================================
//                             Forward declarations
// =============================================================================

    struct SourceDetails;

    class SourceVoiceCallback : public IXAudio2VoiceCallback
    {
    public:
        SourceVoiceCallback(SourceDetails* sourceDetails);
        ~SourceVoiceCallback();

        HANDLE GetBufferEndEvent() const { return m_BufferEndEvent; }

    protected:
        void OnBufferStart(void* pBufferContext);
        void OnBufferEnd(void* pBufferContext);

        void OnVoiceProcessingPassStart(UINT32 bytesRequired) {}
        void OnVoiceProcessingPassEnd() {};

        void OnStreamEnd();
        void OnLoopEnd(void* pBufferContext);

        void OnVoiceError(void* pBufferContext, HRESULT error);
    private:
        SourceDetails* m_SourceDetails;
        HANDLE m_BufferEndEvent;
    };

    struct SourceDetails
    {
        SourceDetails() : Callback(this) {};
        SourceDetails(const SourceDetails&) = default;

        Group_T Group{ CEAL_INVALID_ID };

        uint32_t ChannelCount;
        float CurveDistanceScaler;
        float ChannelRadius;

        float Attributes[SourceAttribute_MaxEnum];

        IXAudio2SourceVoice* XSourceVoice;
        SourceVoiceCallback Callback;

        

        // Streaming
        struct StreamData_
        {
            bool Streaming{ false };
            std::thread Thread;
            const char* Filepath;
            uint32_t BufferMaxSize;
            uint32_t BufferMaxCount;
        } StreamData;
    };

    class ContextDebugger : public IXAudio2EngineCallback
    {
        void OnProcessingPassEnd();
        void OnProcessingPassStart();
        void OnCriticalError(HRESULT error);
    };

    struct GroupDetails
    {
        std::vector<uint32_t> SourceIDs;

        //IXAudio2SubmixVoice* XSubmixVoice;
    };

    struct MasterVoiceDetails {
        XAUDIO2_VOICE_DETAILS Details;
        DWORD ChannelMask;
    };

    struct CealContext {
        // XAudio2 objects
        IXAudio2* XInstance;                                        // Handle to XAudio2's instance
        IXAudio2MasteringVoice* XMasterVoice;                       // Handle to XAudio2's mastering voice
        MasterVoiceDetails MasterDetails;                           // Audio device info / Mastering voice info
        X3DAUDIO_HANDLE X3DInstance;                                // Handle to X3DAudio

        float ListenerAttributes[ListenerAttribute_MaxEnum];        // Listener's space attributes.

        uint32_t GlobalIncrementId;                                 // Global variable keeping track. Increments when a new ceal object is created.
        
        std::unordered_map<uint32_t, XAUDIO2_BUFFER> XBufferMap;    // Buffers
        std::unordered_map<uint32_t, SourceDetails> SourceMap;      // Sources
        std::unordered_map<uint32_t, GroupDetails> GroupMap;        // Groups

        ContextFlags Flags;                                         // Context Flags

        ContextDebugger Debugger;                                   // Debugging

        // Multithreading
        bool IsClosing;
        HANDLE ExitEventHandle;

        void* UserPointer;
    };

    extern inline CealContext* g_CealContext = nullptr;

} // namespace Ceal
