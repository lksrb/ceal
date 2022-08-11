#pragma once

#include "common.h"

// Client API code
namespace Ceal {

	struct AudioFile_Wav;

	CealResult CreateBuffer(Buffer_T* audioBufferId, const AudioFile_Wav* audioFile);
}