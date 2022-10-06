//
// =============================================================================
//                                  CLIENTS API
// =============================================================================
//
#pragma once

#include "ceal_types.h"

// =============================================================================
//                                  Functions
// =============================================================================

/**
 * @brief Creates platform agnostic audio context.
 * @param flags Flags of the context.
 * @return CealResult
 */
CealResult ceal_context_create(CealContextFlags flags = CealContextFlags_None);

/**
 * @brief Creates platform agnostic audio context.
 * @param flags Flags of the context.
 * @param callback Custom allocation callback, search for "ceal_alloc"
 * @return CealResult
 */
CealResult ceal_context_create(CealContextFlags flags, CealAllocationCallback callback); 

/**
 * @brief Destroys platform specific context.
 * @return CealResult
 */
CealResult ceal_context_destroy();

/**
 * @brief Updates Context. Call this function every frame.
 * Note that source's pitch will be overridden when spatial sound is enabled.
 * @return CealResult
 */
CealResult ceal_context_update();

 /**
  * @brief Creates source. Source is usually what you want to store somewhere in order to manipulate audio.
  * @param source ID of the source.
  * @param audioFile Struct populated with audio file information.
  * @param group ID of the group. Zero means source will not be part of any group.
  * @return CealResult
  */
CealResult ceal_source_create(CealSource* source, const CealAudioFile_Wav* audioFile); 

/**
 * @brief Plays submitted buffer on specified source.
 * @param source ID of the source.
 * @return CealResult
 */
CealResult ceal_source_play(CealSource source);

/**
 * @brief Creates buffer from file.
 * @param buffer ID of the buffer.
 * @param audioFile Struct populated with audio file information.
 * @return CealResult
 */
CealResult ceal_buffer_create(CealBuffer* buffer, const CealAudioFile_Wav* audioFile);

/**
 * @brief Submits buffer to specified source.
 * @param source ID of the source.
 * @param buffer ID of the buffer.
 * @return CealResult
 */
CealResult ceal_buffer_submit(CealSource source, CealBuffer buffer);

/**
 * @brief Sets user specific pointer to custom user data.
 * @param userPointer
 */
void ceal_user_pointer_set(void* userPointer);
/**
 * @brief Gets user specific pointer to custom user data.
 * @param userPointer
 */
void ceal_user_pointer_get(void* userPointer);

/**
 * @brief Updates source attribute.
 * @param source ID of the source.
 * @param attribute Attribute to be to changed.
 * @param value New value of the attribute.
 * @return CealResult
 */
void ceal_source_set_float(CealSource source, CealSourceAttribute attribute, float value);

/**
 * @brief Query for source's attribute.
 * @param source ID of the source.
 * @param attribute Source's attribute.
 * @param value Value of the attribute.
 */
void ceal_source_get_float(CealSource source, CealSourceAttribute attribute, float* value);

 /**
  * @brief Sets context flags.
  * @param flags Flags to be changed.
  * @see => ContextFlags_
  */
void ceal_context_set_flags(CealContextFlags flags);

/**
 * @brief Unsets context flags.
 * @param flags Flags to be changed.
 * @see => ContextFlags_
 */
void ceal_context_unset_flags(CealContextFlags flags);

/**
 * @brief Updates listeners attribute.
 * @param attribute Attribute to be changed.
 * @param value New value of the attribute.
 */
void ceal_listener_set_float(CealListenerAttribute attribute, float value);

// =============================================================================
//                                  Streaming
// =============================================================================

// TODO: Streaming is now currently under development

/**
 * @brief Creates audio stream.
 * @param source ID of the source.
 * @param filepath Path to the audio file.
 * @return CealResult
 */
//CealResult ceal_source_stream_create(CealSource source, const char* filepath);

/**
 * @brief Plays audio stream. Do not submit any buffers while streaming.
 * @param source ID of the source.
 * @return CealResult
 */
//CealResult ceal_source_stream_play(CealSource source);
