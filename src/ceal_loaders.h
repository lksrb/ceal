// =============================================================================
//                                Audio Loaders 
// =============================================================================
#pragma once

#include "ceal_types.h"

/**
 * @brief Loads WAVE audio file from disk. Does allocate memory.
 * @param filepath Filepath to audio file.
 * @param audioFile Audio File struct to be populated with audio info.
 * @return CaResult
 */
CealResult ceal_audio_file_wav_load(const char* filepath, CealAudioFile_Wav* audioFile);

/**
 * @brief Reads audio file and populates AudioFile_Wav struct. Does not allocate memory.
 * @param filepath Filepath to audio file.
 * @param audioFile Audio File struct to be populated with audio info.
 * @return CaResult
 */
CealResult ceal_audio_file_wav_get_info(const char* filepath, CealAudioFile_Wav* audioFile);

/**
 * @brief Releases memory from audio file struct. Note that deleting pointer to AudioFile_Wav.Data will also do the trick.
 * @param audioFile Audio File struct populated with audio info.
 * @return CaResult
 */
CealResult ceal_audio_file_wav_free(const CealAudioFile_Wav* audioFile);
