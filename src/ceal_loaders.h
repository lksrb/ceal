//
// =============================================================================
//                                AudioFile Loaders 
// =============================================================================
//
#pragma once

#include "ceal_types.h"

/**
 * @brief Loads WAVE audio file from disk. DOES allocate memory.
 * @param filepath Filepath to audio file.
 * @param audioFile Audio File struct to be populated with audio info.
 * @return CealResult
 */
CealResult ceal_audio_file_wav_load(const char* filepath, CealAudioFile_Wav* audioFile);

/**
 * @brief Releases memory from audio file struct. Note that deleting pointer to AudioFile_Wav.Data will also do the trick.
 * @param audioFile Audio File struct populated with audio info.
 * @return CelaResult
 */
CealResult ceal_audio_file_wav_free(const CealAudioFile_Wav* audioFile);

/**
 * @brief Reads audio file and populates AudioFile_Wav struct. DOES NOT allocate memory.
 * @param filepath Filepath to audio file.
 * @param audioFile Audio File struct to be populated with audio info.
 * @return CealResult
 */
CealResult ceal_audio_file_wav_info(const char* filepath, CealAudioFile_Wav* audioFile);
