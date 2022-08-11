#include "loaders.h"

#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CEAL_RETURN_RESULT(condition, fileStream, result) if(!(condition)) { fclose(fileStream); return result; }

namespace Ceal {

	// Loaders
	CealResult LoadAudioFile_Wav(const char* filepath, AudioFile_Wav* wavFormat) {
		FILE* fileStream = fopen(filepath, "rb");

		if (fileStream == nullptr) 
			return CealResult::CealFileNotFound;
		// Ids
		char riff[5];
		char wave[5];
		char fmt[5];
		char data[5];

		// Zero terminating ids
		riff[4] = '\0';
		wave[4] = '\0';
		fmt[4] = '\0';
		data[4] = '\0';

		// Reading the file
		{
			// RIFF RIFF RIFF
			fread(&riff, 1, 4, fileStream);
			CEAL_RETURN_RESULT(strcmp(riff, "RIFF") == 0, fileStream, CealResult::CealInvalidFormat);

			// ChunkSize
			fread(&wavFormat->ChunkSize, 4, 1, fileStream);

			// WAVE WAVE WAVE
			fread(&wave, 1, 4, fileStream);
			CEAL_RETURN_RESULT(strcmp(wave, "WAVE") == 0, fileStream, CealResult::CealInvalidFormat);

			// FMT
			fread(&fmt, 1, 4, fileStream);
			CEAL_RETURN_RESULT(strcmp(fmt, "fmt ") == 0, fileStream, CealResult::CealInvalidFormat);

			// FMT size
			fread(&wavFormat->FmtSize, 4, 1, fileStream);

			// Format
			fread(&wavFormat->AudioFormat, 2, 1, fileStream);

			// Number of channels
			fread(&wavFormat->NumChannels, 2, 1, fileStream);

			// Samples
			fread(&wavFormat->SampleRate, 4, 1, fileStream);

			// Bytes per seconds
			fread(&wavFormat->ByteRate, 4, 1, fileStream);

			// Block align
			fread(&wavFormat->BlockAlign, 2, 1, fileStream);

			// Bits per sample
			fread(&wavFormat->BitsPerSample, 2, 1, fileStream);

			// DATA DATA DATA
			fread(&data, 1, 4, fileStream);
			CEAL_RETURN_RESULT(strcmp(data, "data") == 0, fileStream, CealResult::CealInvalidFormat);

			// Data size
			fread(&wavFormat->DataSize, 4, 1, fileStream);

			// Allocating buffer
			wavFormat->Data = (uint8_t*)malloc(wavFormat->DataSize);

			// Reading data
			fread(wavFormat->Data, 1, wavFormat->DataSize, fileStream);
		}

		// Closing stream
		fclose(fileStream);

		return CealResult::CealSuccess;
	}
}
