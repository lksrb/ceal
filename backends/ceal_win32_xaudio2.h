#pragma once

#include "ceal_types.h"

// =============================================================================
//									  Functions
// =============================================================================

/**
 * @brief Creates Windows-XAudio2 audio backend.
 * @param flags Flags of the context.
 * @return CealResult
 */
CealResult ceal_backend_win32_xaudio2_init();

/**
 * @brief Destroys Windows-XAudio2 backend.
 * @return CealResult
 */
CealResult ceal_backend_win32_xaudio2_shutdown();
