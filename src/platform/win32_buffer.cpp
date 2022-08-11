// Windows API buffer

#if defined _WIN32 || defined _WIN64

#include "../core/buffer.h"
#include "../core/loaders.h"

#include "win32_core.h"

#include <unordered_map>

namespace Ceal {

	extern CealContext* g_CealContext;

	CealResult CreateBuffer(Buffer_T* bufferId, const AudioFile_Wav* audioFile)
	{
		bufferId->ID = g_CealContext->GlobalIncrementId++;

		// Populating XAUDIO2_BUFFER structure
		auto& buffer = g_CealContext->XBufferMap[*bufferId];
		buffer.AudioBytes = audioFile->DataSize;	// Size of the audio buffer in bytes
		buffer.pAudioData = audioFile->Data;		// Buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM;		// Tell the source voice not to expect any data after this buffer

		return CealResult::CealSuccess;
	}
}

#endif