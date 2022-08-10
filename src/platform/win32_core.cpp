// Windows API buffer
#if defined _WIN32 || defined _WIN64

#include "win32_core.h"
#include "../core/loaders.h"

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
		// Release buffers
		for (const auto&[id, buffer] : g_CealContext->XBufferMap) {
			delete buffer.pAudioData;
		}

		// Release source voices
		for (const auto& [id, sourceVoice] : g_CealContext->XSourceVoiceMap) {
			sourceVoice->Stop();
			sourceVoice->DestroyVoice();
		}

		g_CealContext->XMasterVoice->DestroyVoice();
		g_CealContext->XInstance->Release();

		delete g_CealContext;

		return CealResult::CealSuccess;
	}

	CealResult CreateSource(Source_T* sourceId, const AudioFile_Wav* audioFile)
	{
		(*sourceId) = static_cast<Buffer_T>(g_CealContext->GlobalIncrementId++);

		WAVEFORMATEX wfx = { 0 };
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = audioFile->NumChannels;
		wfx.nSamplesPerSec = audioFile->SampleRate;
		wfx.wBitsPerSample = audioFile->BitsPerSample;
		wfx.cbSize = audioFile->ExtraParamSize;
		wfx.nBlockAlign = audioFile->BlockAlign;
		wfx.nAvgBytesPerSec = audioFile->ByteRate;

		// Create Source Voice
		auto pSourceVoice = &g_CealContext->XSourceVoiceMap[*sourceId];
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSourceVoice(pSourceVoice, &wfx));

		return CealResult::CealSuccess;
	}

	// Submits buffer to a source queue.
	CealResult SubmitBuffer(const Source_T sourceId, const Buffer_T bufferId)
	{
		const auto& buffer = g_CealContext->XBufferMap[bufferId];
		const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
		// Submitting buffer
		CEAL_CHECK_XA2(pSource->SubmitSourceBuffer(&buffer));

		return CealResult::CealSuccess;
	}

	CealResult Play(const Source_T sourceId)
	{
		const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
		// Playing audio
		CEAL_CHECK_XA2(pSource->Start());

		return CealResult::CealSuccess;
	}

	CealResult SetVolume(const Source_T sourceId, float volume)
	{
		if (volume >= 0.0f) {
			const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
			CEAL_CHECK_XA2(pSource->SetVolume(volume));
			return CealResult::CealSuccess;
		}

		return CealResult::CealInvalidValue;
	}
}

#endif