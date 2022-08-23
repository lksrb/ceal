/**
 * 
 * Client API code
 *
 */
#pragma once

#include "ceal_common.h"

namespace Ceal {

// =============================================================================
//									  Queries
// =============================================================================

    /**
     * @brief Query for source's attribute.
     * @param source ID of the source.
     * @param attribute Source's attribute.
     * @param value Value of the attribute.
     */
    void GetSourceFloat(const Source_T* source, SourceAttribute attribute, float* value);

// =============================================================================
//									  Functions
// =============================================================================

	/**
	 * @brief Creates platform specific audio context.
	 * @param flags Flags of the context.
	 * @return Ceal::Result
	 */
	Result CreateContext(ContextFlags flags = ContextFlags_None);

	/**
	 * @brief Destroys platform specific context.
	 * @return Ceal::Result
	 */
	Result DestroyContext();

    /**
     * @brief Sets user specific pointer to data.
     * @param userPointer
     */
    void SetUserPointer(void* userPointer);

    /**
     * @brief Gets user specific pointer to data.
     * @param userPointer
     */
    void GetUserPointer(void* userPointer);

	/**
	 * @brief Creates group.
	 * @param group ID of the group.
	 * @return Ceal::Result
	 */
	Result CreateGroup(Group_T* group);

	/**
	 * @brief Creates source.
	 * @param source ID of the source.
	 * @param audioFile Struct populated with audio file information.
	 * @param group ID of the group. Zero means source will not be part of any group.
	 * @return Ceal::Result
	 */
	Result CreateSource(Source_T* source, const AudioFile_Wav* audioFile, const Group_T* group = nullptr);

	/**
	 * @brief Creates buffer from file.
	 * @param buffer ID of the buffer.
	 * @param audioFile Struct populated with audio file information.
	 * @return Ceal::Result
	 */
	Result CreateBuffer(Buffer_T* buffer, const AudioFile_Wav* audioFile);

	/**
	 * @brief Submits buffer to specified source.
	 * @param source ID of the source.
	 * @param buffer ID of the buffer.
	 * @return Ceal::Result
	 */
	Result SubmitBuffer(const Source_T* source, const Buffer_T* buffer);

	/**
	 * @brief Plays submitted buffer on specified source.
	 * @param source ID of the source.
	 * @return Ceal::Result
	 */
	Result Play(const Source_T* source);

	/**
	 * @brief Plays submitted buffers on specified group of sources.
	 * @param group ID of the group.
	 * @return Ceal::Result
	 */
	Result Play(const Group_T* group);

	/**
	 * @brief Updates source attribute.
	 * @param source ID of the source.
	 * @param attribute Attribute to be to changed.
	 * @param value New value of the attribute.
	 * @return Ceal::Result
	 */
	void SetSourceFloat(const Source_T* source, SourceAttribute attribute, float value);

    /**
     * @brief Updates listeners attribute.
     * @param attribute Attribute to be changed.
     * @param value New value of the attribute.
     */
    void SetListenerFloat(ListenerAttribute attribute, float value);

	/**
	 * @brief Updates group configuration.
	 * @param group ID of the group.
	 * @param config Struct populated with configuration.
	 * @return Ceal::Result
	 */
	//Result SetGroup(const Group_T* group, const GroupConfig* config);

	/**
	 * @brief Sets context flags.
	 * @param flags Flags to be changed.
	 * @see => ContextFlags_
	 */
	void SetContextFlags(ContextFlags flags);

    /**
     * @brief Unsets context flags.
     * @param flags Flags to be changed.
     * @see => ContextFlags_
     */
    void UnsetContextFlags(ContextFlags flags);

    /**
     * @brief Updates Context. Call this function every frame.
     * Note that source's pitch will be overridden when spatial sound is enabled.
     * @return Ceal::Result
     */
    Result Update();

// =============================================================================
//                                  Streaming
// =============================================================================

	/**
	 * @brief Creates audio stream.
	 * @param source ID of the source.
	 * @param filepath Path to the audio file.
	 * @return Ceal::Result
	 */
	Result CreateStream(const Source_T* source, const char* filepath);

	/**
	 * @brief Plays audio stream. Do not submit any buffers while streaming.
	 * @param source ID of the source.
	 * @return Ceal::Result
	 */
	Result PlayStream(const Source_T* source);
}
