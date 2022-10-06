#include "ceal.h"

#include "ceal_common.h"
#include "ceal_debug.h"
#include "ceal_internal.h"

#include <thread>

// =============================================================================
//									  Macros
// =============================================================================
#define CEAL_PI 3.141592654f

#define CEAL_CHOOSE_CHANNEL_AZIMUTHS(__input)   const_cast<float*>(s_ChannelAzimuths[__input])
#define CEAL_SOURCE_AT(__sourceId)              g_GlobalContext->SourceMap.at(*__sourceId)

extern CealGlobalContext* g_GlobalContext = nullptr;
extern CealAllocationCallback g_AllocationCallback = nullptr;

/**
 * @brief Default allocation function for CEAL. It is possible and preferred that you create your own allocation function. 
 * @param ptr Pointer to object, used for deallocation/reallocation
 * @param oldsize Old size of the object
 * @param newsize New size of the object
 * @return void* Pointer to object  
 */
static void* ceal_alloc(void* ptr, size_t oldsize, size_t newsize)
{
    // Deallocation
    if (newsize == 0)
    {
        free(ptr);
        return nullptr;
    }
    else
    {
        return realloc(ptr, newsize);
    }
}
// =============================================================================
//									  Functions
// =============================================================================

/**
 * @brief Creates platform agnostic audio context.
 */
CealResult ceal_context_create(CealContextFlags flags)
{
    return ceal_context_create(flags, ceal_alloc);
}

/**
 * @brief Creates platform agnostic audio context with custom allocation function.
 */
CealResult ceal_context_create(CealContextFlags flags, CealAllocationCallback callback)
{
    CEAL_ASSERT(callback); // Callback cannot be nullptr!

#ifdef CEAL_DEBUG 
    ceal_debug_init();
#endif
    // Assign callback function
    g_AllocationCallback = callback;

    // Create global context
    g_GlobalContext = CEAL_NEW(CealGlobalContext);

    // Allocating memory for arrays
    g_GlobalContext->SourceBase = CEAL_MEM_ALLOC_SIZE(CealSource, sizeof(CealSource) * 5);
    memset(g_GlobalContext->SourceBase, 0, sizeof(CealSource) * 5);

    g_GlobalContext->BufferBase = CEAL_MEM_ALLOC_SIZE(CealBuffer, sizeof(CealSource) * 5);
    memset(g_GlobalContext->BufferBase, 0, sizeof(CealSource) * 5);

    //g_GlobalContext->ThreadPool = CEAL_MEM_ALLOC_SIZE(std::thread, sizeof(std::thread) * 5);

    // Initiliaze XListener to default values
    memset(g_GlobalContext->ListenerAttributes, 0, sizeof(g_GlobalContext->ListenerAttributes));
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientFrontX] = 1.0f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_OrientTopZ] = 1.0f;

    // From x3daudio.h => X3DAudioDefault_DirectionalCone
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerAngle] = CEAL_PI / 2; CEAL_PI, 1.0f, 0.708f, 0.0f, 0.25f, 0.708f, 1.0f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterAngle] = CEAL_PI;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerVolume] = 1.0f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterVolume] = 0.708f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerLPF] = 0.0f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterLPF] = 0.25f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_InnerReverb] = 0.708f;
    g_GlobalContext->ListenerAttributes[CealListenerAttribute_3DCone_OuterReverb] = 1.0f;

    // Assign global flags
    g_GlobalContext->Flags = flags;

    // For multithread purposes
    g_GlobalContext->IsClosing = false;

    return CealResult_Success;
}

/**
 * @brief Destroys platform specific context.
 */
CealResult ceal_context_destroy()
{
    CEAL_ASSERT(g_GlobalContext->Backend == nullptr);

    // Wait for threads
    for (auto& thread : g_GlobalContext->ThreadPool)
    {
        while (thread.joinable() == false)
        {
        }
        thread.join();
    }
  /*  for (size_t i = 0; i < g_GlobalContext->ThreadPoolSize; ++i)
    {
        while(g_GlobalContext->ThreadPool[i].joinable() == false)
        {
        }
        g_GlobalContext->ThreadPool[i].join();
        g_GlobalContext->ThreadPool[i].~thread();
    }

    CEAL_MEM_FREE(g_GlobalContext->ThreadPool);*/

    for (size_t i = 0; i < g_GlobalContext->SourceSize; ++i)
    {
        CealSource currentSource = g_GlobalContext->SourceBase[i];

        // Release stream buffers 
        for (uint32_t i = 0; i < currentSource->StreamData.BufferMaxCount; ++i)
        {
            CEAL_MEM_ARRAY_FREE(currentSource->StreamData.Buffers[i], currentSource->StreamData.BufferMaxSize);
        }
        CEAL_MEM_ARRAY_FREE(currentSource->StreamData.Buffers, currentSource->StreamData.BufferMaxCount);

        // Release source
        CEAL_MEM_FREE(currentSource);
    }

    for (size_t i = 0; i < g_GlobalContext->BufferSize; ++i)
    {
        CEAL_MEM_FREE(g_GlobalContext->BufferBase[i]);
    }

    CEAL_MEM_FREE(g_GlobalContext->SourceBase);
    CEAL_MEM_FREE(g_GlobalContext->BufferBase);

   /* // Release sources
    for (auto& source : g_GlobalContext->Sources)
    {
        CEAL_MEM_FREE(source);
    }*/

   /* // Release buffers
    for (auto& buffer : g_GlobalContext->Buffers)
    {
        CEAL_MEM_FREE(buffer);
    }*/

    // Release global context
    CEAL_DELETE(CealGlobalContext, g_GlobalContext);

#ifdef CEAL_DEBUG 
    ceal_debug_check_memory_leaks();
#endif
    return CealResult_Success;
}

/**
 * @brief Creates buffer from file.
 */
CealResult ceal_buffer_create(CealBuffer* buffer, const CealAudioFile_Wav* audioFile)
{
    // Populating XAUDIO2_BUFFER structure
    *buffer = CEAL_NEW(CealBuffer_T);

    g_GlobalContext->BufferBase[g_GlobalContext->BufferSize++] = *buffer;

    auto& bufferPtr = *buffer;
    bufferPtr->Size = audioFile->DataSize;	    // Size of the audio buffer in bytes
    bufferPtr->Buffer = audioFile->Data;		// Buffer containing audio data

    return ceal_internal_buffer_create(bufferPtr, audioFile);
}

/**
 * @brief Creates source.
 */
CealResult ceal_source_create(CealSource* source, const CealAudioFile_Wav* audioFile)
{
    // Create Source Voice
    *source = CEAL_NEW(CealSource_T);

    g_GlobalContext->SourceBase[g_GlobalContext->SourceSize++] = *source;

    auto& sourcePtr = *source;
    // Default values for emitter attributes
    sourcePtr->ChannelCount = audioFile->NumChannels; // TODO(Urby): Figure out how to implement multichannel
    memset(sourcePtr->Attributes, 0, sizeof(sourcePtr->Attributes));
    sourcePtr->Attributes[CealSourceAttribute_Volume] = 1.0f;
    sourcePtr->Attributes[CealSourceAttribute_Pitch] = 1.0f;
    sourcePtr->Attributes[CealSourceAttribute_3D_ChannelRadius] = 1.0f;
    sourcePtr->Attributes[CealSourceAttribute_3D_CurveDistanceScaler] = 1.0f;

    // Creates audio backend specific source
    if (ceal_internal_source_create(sourcePtr, audioFile) == CealResult_Failed)
    {
        __debugbreak();
        return CealResult_Failed;
    }

    return CealResult_Success;
}

/**
 * @brief Plays submitted buffer on specified source.
 */
CealResult ceal_source_play(CealSource source)
{
    source->Occupied = true;
    return ceal_internal_source_play(source);
}

/**
 * @brief Submits buffer to specified source.
 */
CealResult ceal_buffer_submit(CealSource source, CealBuffer buffer)
{
    return ceal_internal_buffer_submit(source, buffer);
}

/**
 * @brief Sets context flags.
 */
void ceal_context_set_flags(CealContextFlags flags)
{
    g_GlobalContext->Flags |= flags;
}

/**
 * @brief Unsets context flags.
 */
void ceal_context_unset_flags(CealContextFlags flags)
{
    g_GlobalContext->Flags &= ~flags;
}

/**
 * @brief Sets user specific pointer to data.
 */
void ceal_user_pointer_set(void* userPointer)
{
    g_GlobalContext->UserPointer = userPointer;
}

/**
 * @brief Gets user specific pointer to data.
 */
void ceal_user_pointer_get(void* userPointer)
{
    userPointer = g_GlobalContext->UserPointer;
}

/**
 * @brief Queries for source's attribute.
 */
void ceal_source_get_float(CealSource source, CealSourceAttribute attribute, float* value)
{
    value = &source->Attributes[attribute];
}

/**
 * @brief Updates source attributes.
 */
void ceal_source_set_float(CealSource source, CealSourceAttribute attribute, float value)
{
    source->Attributes[attribute] = value;
}

/**
 * @brief Updates listeners attribute.
 */
void ceal_listener_set_float(CealListenerAttribute attribute, float value)
{
    g_GlobalContext->ListenerAttributes[attribute] = value;
}

/**
 * @brief Updates Context. Call this function every frame.
 */
CealResult ceal_context_update()
{
    CEAL_ASSERT(ceal_internal_context_update());
    return CealResult_Success;
}

// =============================================================================
//                                  Streaming                                   
// =============================================================================

/**
 * @brief Creates asynchronous audio stream.
 */
CealResult ceal_source_stream_create(CealSource source, const char* filepath)
{
    CEAL_ASSERT(filepath); // Invalid filepath!

    source->StreamData.Filepath = filepath;
    source->StreamData.BufferMaxCount = 3;
    source->StreamData.BufferMaxSize = 65536;

    uint8_t** buffers = CEAL_MEM_ARRAY_ALLOC(uint8_t*, source->StreamData.BufferMaxCount);

    for (uint32_t i = 0; i < source->StreamData.BufferMaxCount; ++i)
    {
        buffers[i] = CEAL_MEM_ARRAY_ALLOC(uint8_t, source->StreamData.BufferMaxSize);
    }

    source->StreamData.Buffers = buffers;

    return CealResult_Success;
}

/**
 * @brief Plays audio stream. Do not submit any buffers while streaming.
 */
CealResult ceal_source_stream_play(CealSource source)
{
    CEAL_VERIFY(source->StreamData.IsStreaming); // Stream is already playing!
    CEAL_VERIFY(source->Occupied); // Source is already playing loaded audio!

   /* auto& stream = g_GlobalContext->ThreadPool[g_GlobalContext->ThreadPoolSize++];
    new(&stream)std::thread();*/

    auto& stream = g_GlobalContext->ThreadPool.emplace_back();

    // Starting the thread 
    stream = std::thread([source]()
    {
        ceal_internal_stream_function(source);

        // Notify main thread that this thread is done.
        source->StreamData.IsStreaming = false;
    });
    source->StreamData.IsStreaming = true;
    return CealResult_Success;
}
