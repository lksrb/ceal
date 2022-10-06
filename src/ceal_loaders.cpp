#include "ceal_loaders.h"

#include "ceal_common.h"
#include "ceal_debug.h"

#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CEAL_RESULT_RETURN(condition, fileStream, result) if(!(condition)) { fclose(fileStream); return result; }

/**
 * @brief Loads WAVE audio file from disk. Does allocate memory.
 */
CealResult ceal_audio_file_wav_load(const char* filepath, CealAudioFile_Wav* audioFile)
{
    CEAL_MEASURE_FUNCTION();

    FILE* fileStream = fopen(filepath, "rb");

    if (fileStream == nullptr)
        return CealResult_Failed;
    // Ids
    char magic[5];

    // Zero terminating to be able to compare two strings easily
    magic[4] = '\0';

    // Reading the file
    fread(&magic, 4, 1, fileStream);                            // Riff section
    CEAL_RESULT_RETURN(strcmp(magic, "RIFF") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->ChunkSize, 4, 1, fileStream);             // Chunk size
    fread(&magic, 4, 1, fileStream);                            // Wave section
    CEAL_RESULT_RETURN(strcmp(magic, "WAVE") == 0, fileStream, CealResult_InvalidFormat);
    fread(&magic, 4, 1, fileStream);                            // FMT section
    CEAL_RESULT_RETURN(strcmp(magic, "fmt ") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->FmtSize, 4, 1, fileStream);               // FMT size
    fread(&audioFile->AudioFormat, 2, 1, fileStream);           // Format
    fread(&audioFile->NumChannels, 2, 1, fileStream);           // Number of channels
    fread(&audioFile->SampleRate, 4, 1, fileStream);            // Samples
    fread(&audioFile->ByteRate, 4, 1, fileStream);              // Bytes per seconds
    fread(&audioFile->BlockAlign, 2, 1, fileStream);            // Block align
    fread(&audioFile->BitsPerSample, 2, 1, fileStream);         // Bits per sample
    fread(&magic, 4, 1, fileStream);                            // Data chunk
    CEAL_RESULT_RETURN(strcmp(magic, "data") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->DataSize, 1, 4, fileStream);              // Data size
    fgetpos(fileStream, (fpos_t*)&audioFile->DataOffset);       // Data offset for streaming

    // Allocating buffer
    audioFile->Data = (uint8_t*)malloc(audioFile->DataSize);

    fread(audioFile->Data, 1, audioFile->DataSize, fileStream); // Reading data

    // Closing stream
    fclose(fileStream);

    return CealResult_Success;
}

/**
 * @brief Releases memory from audio file struct. Note that deleting pointer to AudioFile_Wav.Data will also do the trick.
 */
CealResult ceal_audio_file_wav_free(const CealAudioFile_Wav* audioFile)
{
    CEAL_ASSERT(audioFile);         // Invalid audio file!
    CEAL_ASSERT(audioFile->Data);   // Data do not point to any allocated memory! Note: ceal_audio_file_wav_get_info(...) do not allocate memory.
    delete audioFile->Data;

    return CealResult_Success;
}

/**
 * @brief Reads audio file and populates AudioFile_Wav struct. Does not allocate memory.
 */
CealResult ceal_audio_file_wav_info(const char* filepath, CealAudioFile_Wav* audioFile)
{
    FILE* fileStream = fopen(filepath, "rb");

    if (fileStream == nullptr)
        return CealResult_FileNotFound;
    // Ids
    char magic[5];

    // Zero terminating to be able to compare two strings easily
    magic[4] = '\0';

    // Reading the file
    fread(&magic, 4, 1, fileStream);                            // Riff section
    CEAL_RESULT_RETURN(strcmp(magic, "RIFF") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->ChunkSize, 4, 1, fileStream);             // Chunk size
    fread(&magic, 4, 1, fileStream);                            // Wave section
    CEAL_RESULT_RETURN(strcmp(magic, "WAVE") == 0, fileStream, CealResult_InvalidFormat);
    fread(&magic, 4, 1, fileStream);                            // FMT section
    CEAL_RESULT_RETURN(strcmp(magic, "fmt ") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->FmtSize, 4, 1, fileStream);               // FMT size
    fread(&audioFile->AudioFormat, 2, 1, fileStream);           // Format
    fread(&audioFile->NumChannels, 2, 1, fileStream);           // Number of channels
    fread(&audioFile->SampleRate, 4, 1, fileStream);            // Samples
    fread(&audioFile->ByteRate, 4, 1, fileStream);              // Bytes per seconds
    fread(&audioFile->BlockAlign, 2, 1, fileStream);            // Block align
    fread(&audioFile->BitsPerSample, 2, 1, fileStream);         // Bits per sample
    fread(&magic, 4, 1, fileStream);                            // Data chunk
    CEAL_RESULT_RETURN(strcmp(magic, "data") == 0, fileStream, CealResult_InvalidFormat);
    fread(&audioFile->DataSize, 4, 1, fileStream);              // Data size
    fgetpos(fileStream, (fpos_t*)&audioFile->DataOffset);       // Data offset for streaming

    // Closing stream
    fclose(fileStream);

    return CealResult_Success;
}

