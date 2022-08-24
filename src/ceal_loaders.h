// =============================================================================
//                                Audio Loaders 
// =============================================================================
#pragma once

#include "ceal_common.h"

namespace CEAL {
	
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
