#pragma once

#include "common.h"

// Client API code
namespace ceal {

	struct AudioFile_Wav;

	// Default listener
	struct AudioListenerConfig
	{
		float Position[3] = { 0.0f, 0.0f, 0.0f };
		float Velocity[3] = { 0.0f, 0.0f, 0.0f };
		float ListenerOrientation[6] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	};

	struct AudioSourceConfig
	{
		float Position[3] = { 0.0f, 0.0f, 1.0f };
		float Velocity[3] = { 0.0f, 0.0f, 0.0f };
		float Gain = 1.0f;
		float Pitch = 1.0f;
		bool Loop = false;
		bool Spacial = false;
	};

	// Creates platform specific context
	CealResult CreateContext();

	// Destroys context
	CealResult DestroyContext();

	// Creates source
	CealResult CreateSource(Source_T* sourceId, const AudioFile_Wav* audioFile);

	// Submit buffer to specific source
	CealResult SubmitBuffer(const Source_T sourceId, const Buffer_T bufferId);

	// Starts the queue
	CealResult Play(const Source_T sourceId);

	// Volume configuration
	CealResult SetVolume(const Source_T sourceId, float volume);
}