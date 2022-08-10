
#include "core/core.h"
#include "core/loaders.h"
#include "core/buffer.h"

#include <iostream>

#include <xaudio2.h>
#include <assert.h>
#include <conio.h>

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

		ceal::Buffer_T audioId;
		ceal::Source_T sourceId;

		ceal::CreateAudioBuffer(&audioId, &audioFile);
		ceal::CreateSource(&sourceId, &audioFile);
		ceal::SetVolume(sourceId, 0.05f);

		ceal::SubmitBuffer(sourceId, audioId);

		ceal::Play(sourceId); 

		bool runForever = true;
		while (runForever) {
			char c;

			static float volume = 0.05f;

			std::cout << "What would you like to do?\n";
			std::cout << "(e) Exit\n";
			std::cout << "(+) Increase volume\n";
			std::cout << "(-) Decrease volume\n";

			c = _getch();
			switch (c)
			{
				case 'e': 
				{
					runForever = false;
					break; 
				}
				case '+': 
				{
					volume += 0.05f;
					ceal::SetVolume(sourceId, volume);
					std::cout << "Volume: " << volume << "\n";
					break; 
				}
				case '-': 
				{
					volume -= 0.05f;
					ceal::SetVolume(sourceId, volume);
					std::cout << "Volume: " << volume << "\n";
					break; 
				}
				default:
					break;
			}
		}


		ceal::DestroyContext();
	}
	return 0;
}
