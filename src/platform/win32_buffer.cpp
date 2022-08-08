// Windows API buffer

#if defined _WIN32 || defined _WIN64

#include "../core/buffer.h"
#include "../core/loaders.h"

#include "win32_core.h"

namespace ceal {

	extern CealContext* g_CealContext;

	CealResult CreateAudioBuffer(AudioBufferID* audioBufferId, const AudioFile_Wav* audioFile)
	{
		// Populating XAUDIO2_BUFFER structure
		auto& buffer = g_CealContext->XBuffers.emplace_back();
		buffer.AudioBytes = audioFile->DataSize;  //size of the audio buffer in bytes
		buffer.pAudioData = audioFile->Data;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		WAVEFORMATEX wfx = { 0 };
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = audioFile->NumChannels;
		wfx.nSamplesPerSec = audioFile->SampleRate;
		wfx.wBitsPerSample = audioFile->BitsPerSample;
		wfx.cbSize = audioFile->ExtraParamSize;
		wfx.nBlockAlign = audioFile->BlockAlign;
		wfx.nAvgBytesPerSec = audioFile->ByteRate;

		// Create Source Voice
		IXAudio2SourceVoice** pSourceVoice = &g_CealContext->XSourceVoices.emplace_back();
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSourceVoice(pSourceVoice, &wfx));

		// Submitting buffer
		CEAL_CHECK_XA2((*pSourceVoice)->SubmitSourceBuffer(&buffer));

		// Playing audio
		(*pSourceVoice)->Start();

		return CealResult::CealSuccess;
	}

}
#endif