#pragma once

#include "common.h"

namespace ceal {

	/*
		Based on http://soundfile.sapp.org/doc/WaveFormat/
	*/

	struct AudioFile_Wav {
		uint32_t ChunkSize;

		uint32_t FmtSize; // 16 for PCM
		uint16_t AudioFormat;
		uint16_t NumChannels;
		uint32_t SampleRate;
		uint32_t ByteRate;
		uint16_t BlockAlign;
		uint16_t BitsPerSample;
		// if not PCM
		uint16_t ExtraParamSize;

		uint32_t DataSize;
		uint8_t* Data;

		AudioFile_Wav() 
		{
			memset(this, 0, sizeof(AudioFile_Wav));
		}
	};

	// Loaders
	CealResult LoadAudioFile_Wav(const char* filepath, AudioFile_Wav* info);
}