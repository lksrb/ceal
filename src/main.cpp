
#include "core/core.h"
#include "core/loaders.h"
#include "core/buffer.h"

#include <iostream>

#include <assert.h>
#include <conio.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define CEAL_ASSERT(condition, msg) if((condition) == Ceal::CealResult::CealFailed) { std::cerr << msg << "\n"; __debugbreak(); }

// We do not talk about it
#if 0
static void X3DTest() {
	{
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		Ceal::CreateContext();

		//TODO(Urb):  Figure out how it actually works
		Ceal::AudioFile_Wav audioFile;
		CEAL_ASSERT(Ceal::LoadAudioFile_Wav("testing/file_example_WAV_2MG.wav", &audioFile), "Failed to load file!");

		Ceal::Buffer_T audioId;
		Ceal::Source_T sourceId;

		Ceal::CreateAudioBuffer(&audioId, &audioFile);
		Ceal::CreateSource(&sourceId, &audioFile);
		Ceal::SetVolume(sourceId, 0.05f);

		// Setup audio listener and sources
		// Setup audio listener and sources
		// Setup audio listener and sources
		Ceal::AudioListenerConfigV2 listenerConfig;

		listenerConfig.Position[0] = { 0.0f };
		listenerConfig.Position[1] = { 0.0f };
		listenerConfig.Position[2] = { 0.0f };

		listenerConfig.Velocity[0] = { 0.0f };
		listenerConfig.Velocity[1] = { 0.0f };
		listenerConfig.Velocity[2] = { 0.0f };

		listenerConfig.OrientFront[0] = { 0.0f };  // X
		listenerConfig.OrientFront[1] = { 0.0f };  // Y
		listenerConfig.OrientFront[2] = { -1.0f }; // Z

		listenerConfig.OrientTop[0] = { 0.0f }; // X
		listenerConfig.OrientTop[1] = { 1.0f };	// Y
		listenerConfig.OrientTop[2] = { 0.0f };	// Z

		Ceal::AudioSourceConfig sourceConfig;

		Ceal::ConfigureAudioSource(sourceId, &sourceConfig);
		Ceal::ConfigureAudioListener(&listenerConfig);

		Ceal::Update(sourceId);

		// Submitting

		Ceal::SubmitBuffer(sourceId, audioId);
		Ceal::Play(sourceId);

		// Test loop
		// Test loop
		// Test loop
		bool runForever = true;
		while (runForever) {
			char c = 'f';

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
				Ceal::SetVolume(sourceId, volume);
				std::cout << "Volume: " << volume << "\n";
				break;
			}
			case '-':
			{
				volume -= 0.05f;
				Ceal::SetVolume(sourceId, volume);
				std::cout << "Volume: " << volume << "\n";
				break;
			}

			case 'w':
			{
				listenerConfig.Position[1] += 1.0f; // Moving forward
				break;
			}
			case 's':
			{
				listenerConfig.Position[1] -= 1.0f; // Moving backward
				break;
			}
			case 'a':
			{
				listenerConfig.Position[0] -= 1.0f; // Moving left
				break;
			}
			case 'd':
			{
				listenerConfig.Position[0] += 1.0f; // Moving right
				break;
			}
			default:
				break;
			}

			Ceal::ConfigureAudioListener(&listenerConfig);
			Ceal::Update(sourceId);
		}


		Ceal::DestroyContext();
	}
}
#endif

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

		Ceal::CreateContext();

		//TODO(Urb):  Figure out how it actually works
		Ceal::AudioFile_Wav audioFile;
		CEAL_ASSERT(Ceal::LoadAudioFile_Wav("testing/file_example_WAV_2MG.wav", &audioFile), "Failed to load file!");

		Ceal::Buffer_T bufferId;
		Ceal::Source_T sourceId;

		// Group source audios together to manage them all
		Ceal::CreateBuffer(&bufferId, &audioFile);
		Ceal::CreateSource(&sourceId, &audioFile);
		Ceal::SetVolume(sourceId, 0.05f);

		// Submitting
		Ceal::SubmitBuffer(sourceId, bufferId);
		Ceal::Play(sourceId);

		// Test loop
		// Test loop
		// Test loop
		bool runForever = true;
		while (runForever) {
			char c = 'f';

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
				Ceal::SetVolume(sourceId, volume);
				std::cout << "Volume: " << volume << "\n";
				break;
			}
			case '-':
			{
				volume -= 0.05f;
				Ceal::SetVolume(sourceId, volume);
				std::cout << "Volume: " << volume << "\n";
				break;
			}

			default:
				break;
			}
		}


		Ceal::DestroyContext();
	}
	return 0;
}
