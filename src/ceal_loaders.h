#pragma once

#include "ceal_common.h"

namespace Ceal {
	
	/**
	 * @brief Holds info about Wav audio
	 * @see http://soundfile.sapp.org/doc/WaveFormat/
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
		
		// Data
		uint32_t DataSize;
        uint32_t DataOffset;
        uint8_t* Data;
		
		AudioFile_Wav() 
		{
			memset(this, 0, sizeof(AudioFile_Wav));
		}
	};
	
	/**
	 * @brief Loads WAVE audio file from disk. Does allocate memory.
	 * @param filepath Filepath to audio file.
	 * @param audioFile Audio File struct to be populated with audio info.
	 * @return Ceal::Result
	 */
	Result LoadAudioFile_Wav(const char* filepath, AudioFile_Wav* audioFile);

    /**
     * @brief Reads audio file and populates AudioFile_Wav struct. Does not allocate memory.
     * @param filepath Filepath to audio file.
     * @param audioFile Audio File struct to be populated with audio info.
     * @return Ceal::Result
     */
    Result GetAudioFileInfo_Wav(const char* filepath, AudioFile_Wav* audioFile);
}
