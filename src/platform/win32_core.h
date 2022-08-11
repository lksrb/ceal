#pragma once

#include "../core/core.h"

#include <vector>
#include <unordered_map>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib") 

#define CEAL_CHECK_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { return CealResult::CealFailed;  }  }

namespace Ceal {

	struct CealContext {
		// XAudio objects
		IXAudio2* XInstance;
		IXAudio2MasteringVoice* XMasterVoice;
		IXAudio2SubmixVoice* XSubmitVoice;

		// Audio device info

		// X3D objects
		X3DAUDIO_HANDLE X3DInstance;

		// Keep track of everything 
		uint32_t GlobalIncrementId{ 0 };
		std::unordered_map<uint32_t, XAUDIO2_BUFFER> XBufferMap;
		std::unordered_map<uint32_t, IXAudio2SourceVoice*> XSourceVoiceMap;

		// Listener/Source relationship used in game
		X3DAUDIO_LISTENER GListener;
		std::unordered_map<uint32_t, X3DAUDIO_EMITTER> GAudioSourceMap;
	};
}