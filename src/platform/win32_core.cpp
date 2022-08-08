// Windows API buffer
#if defined _WIN32 || defined _WIN64

#include "win32_core.h"

namespace ceal {

	extern CealContext* g_CealContext = nullptr;

	CealResult CreateContext()
	{
		g_CealContext = new CealContext;

		// Create COM
		CEAL_CHECK_XA2(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

		// Create audio engine instance 
		CEAL_CHECK_XA2(XAudio2Create(&g_CealContext->XInstance, 0, XAUDIO2_DEFAULT_PROCESSOR));

		// Create mastering voice
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateMasteringVoice(&g_CealContext->XMasterVoice));

		return CealResult::CealSuccess;
	}

	CealResult DestroyContext()
	{
		// Releasing buffers
		for (const auto& buffer : g_CealContext->XBuffers) {
			delete buffer.pAudioData;
		}

		// Releasing source voices
		for (auto sourceVoice : g_CealContext->XSourceVoices) {
			sourceVoice->Stop();
			sourceVoice->DestroyVoice();
		}

		g_CealContext->XMasterVoice->DestroyVoice();
		g_CealContext->XInstance->Release();

		delete g_CealContext;

		return CealResult::CealSuccess;
	}
}

#endif