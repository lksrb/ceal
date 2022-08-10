#pragma once

#include "../core/core.h"

#include <vector>
#include <unordered_map>

#include <xaudio2.h>

#define CEAL_CHECK_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { return CealResult::CealFailed;  }  }

namespace ceal {

	struct CealContext {
		IXAudio2* XInstance;
		IXAudio2MasteringVoice* XMasterVoice;

		// Keep track of everything 
		uint32_t GlobalIncrementId{ 0 };
		std::unordered_map<Buffer_T, XAUDIO2_BUFFER> XBufferMap;
		std::unordered_map<Source_T, IXAudio2SourceVoice*> XSourceVoiceMap;
	};
}