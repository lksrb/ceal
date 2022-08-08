
#include "core/core.h"
#include "core/loaders.h"
#include "core/buffer.h"

#include <iostream>

#include <xaudio2.h>
#include <assert.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define CEAL_CHECK_XA2(x) { HRESULT hr = (x); if(hr != S_OK) { printf("%s failed with code: %d", #x, (int)hr); __debugbreak();}  }

#define CEAL_ASSERT(condition, msg) if((condition) == ceal::CealResult::CealFailed) { std::cerr << msg << "\n"; __debugbreak(); }

int main()
{
	{
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		ceal::CreateContext();

		//TODO(Urb):  Figure out how it actually works
		ceal::AudioFile_Wav audioFile;
		CEAL_ASSERT(ceal::LoadAudioFile_Wav("testing/file_example_WAV_2MG.wav", &audioFile), "Failed to load file!");

		ceal::AudioBufferID audioId;

		ceal::CreateAudioBuffer(&audioId, &audioFile);

		std::cin.get();

		ceal::DestroyContext();
	}
	return 0;
}
