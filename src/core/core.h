#pragma once

#include "common.h"
// Client API code
/**
 * 
 * 
 * 
 */
namespace Ceal {

	struct AudioFile_Wav;

	// Default listener
	struct AudioListenerConfig
	{
		float Position[3] = { 0.0f, 0.0f, 0.0f };
		float Velocity[3] = { 0.0f, 0.0f, 0.0f };
		float ListenerOrientation[6] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	};

	struct AudioListenerConfigV2
	{
		float Position[3];
		float Velocity[3];
		float OrientFront[3];
		float OrientTop[3];
	};

	struct AudioSourceConfig
	{
		float Position[3] = { 0.0f, 0.0f, 0.0f };
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

	// Listener/Source relationship used in game
	 
	// Audio Listener
	void ConfigureAudioListener(const AudioListenerConfigV2* config);

	// Audio Source
	void ConfigureAudioSource(const Source_T sourceid, const AudioSourceConfig* config);

	// Internal
	void Update(const Source_T sourceId);
}