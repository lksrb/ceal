#pragma once

#include "ceal_types.h"

#include <unordered_map>
#include <thread>

// =============================================================================
//					                 Declaration
// =============================================================================

typedef struct CealSource_T
{
    float Attributes[CealSourceAttribute_MaxEnum];
    uint32_t ChannelCount;
    bool Occupied;

    // Streaming
    struct StreamData_
    {
        bool IsStreaming{ false };
        const char* Filepath;
        uint32_t BufferMaxSize;
        uint32_t BufferMaxCount;
        
        uint8_t** Buffers;
        void* StreamCallback;
        void* BufferEndEvent;
    } StreamData;
} *CealSource;

typedef struct CealBuffer_T
{
    void* Buffer;
    size_t Size;
} *CealBuffer;

// =============================================================================
//					                 Macros
// =============================================================================

#define CEAL_INVALID_ID 0

/**
 * @brief Connects global context with audio backend.
 */
struct CealGlobalContext
{
    float ListenerAttributes[CealListenerAttribute_MaxEnum];      // Listener's space attributes.

    //uint32_t GlobalIncrementId;                                 // Global variable keeping track. Increments when a new ceal object is created.

    CealContextFlags Flags = CealContextFlags_None;               // Context flags

    bool IsClosing{ false };                                      // Multithreading

    void* UserPointer{ nullptr };                                 // Pointer that can be set by user/client.

    void* Backend{ nullptr };                                     // Current audiobackend

    CealSource* SourceBase = nullptr;
    uint32_t SourceSize = 0;

    CealBuffer* BufferBase = nullptr;
    uint32_t BufferSize = 0;

    std::vector<std::thread> ThreadPool;
    //std::thread* ThreadPool; // TODO: Thread pool
    //size_t ThreadPoolSize;

    //HANDLE ExitEventHandle;
};

// =============================================================================
//		   Internal functions to be implemented by chosen audio backend.
// =============================================================================

CealResult ceal_internal_source_play(CealSource source);

CealResult ceal_internal_buffer_submit(CealSource source, CealBuffer buffer);

CealResult ceal_internal_source_create(CealSource source, const CealAudioFile_Wav* audioFile);

CealResult ceal_internal_buffer_create(CealBuffer buffer, const CealAudioFile_Wav* audioFile);

CealResult ceal_internal_context_update();

void ceal_internal_stream_function(CealSource source);
