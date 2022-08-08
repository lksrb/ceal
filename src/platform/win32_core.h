#pragma once

#include "../core/core.h"

#include <vector>

#include <xaudio2.h>

#define CEAL_CHECK_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { return CealResult::CealFailed;  }  }

namespace ceal {

	struct CealContext {

		IXAudio2* XInstance;
		IXAudio2MasteringVoice* XMasterVoice;

		std::vector<XAUDIO2_BUFFER> XBuffers;
		std::vector<IXAudio2SourceVoice*> XSourceVoices;
	};
}