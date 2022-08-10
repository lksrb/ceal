#pragma once

#include "common.h"

// Client API code
namespace ceal {

	struct AudioFile_Wav;

	CealResult CreateAudioBuffer(Buffer_T* audioBufferId, const AudioFile_Wav* audioFile);
}