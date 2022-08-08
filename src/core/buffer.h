#pragma once

#include "common.h"

// Client API code
namespace ceal {

	struct AudioFile_Wav;

	CealResult CreateAudioBuffer(AudioBufferID* audioBufferId, const AudioFile_Wav* audioFile);
}