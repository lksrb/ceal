/**
 *
 * Ceal types
 *
 */
#pragma once

#include <stdint.h> // uint32_t
#include <vcruntime_string.h> // memset, ...

enum CealResult_ : uint32_t
{
    CealResult_Failed = 0,
    CealResult_Success,
    CealResult_FileNotFound,
    CealResult_InvalidFormat,
    CealResult_InvalidValue,
};

enum CaContextFlags_ : uint32_t
{
    CealContextFlags_None = 0,
    CealContextFlags_Enable3D = 1 << 0,
};

enum CaSourceAttribute_ : uint32_t
{
    CealSourceAttribute_Volume = 0,
    CealSourceAttribute_Pitch,
    CealSourceAttribute_3D_ChannelRadius,
    CealSourceAttribute_3D_CurveDistanceScaler,

    CealSourceAttribute_OrientFrontX,
    CealSourceAttribute_OrientFrontY,
    CealSourceAttribute_OrientFrontZ,
    CealSourceAttribute_OrientTopX,
    CealSourceAttribute_OrientTopY,
    CealSourceAttribute_OrientTopZ,
    CealSourceAttribute_PositionX,
    CealSourceAttribute_PositionY,
    CealSourceAttribute_PositionZ,
    CealSourceAttribute_VelocityX,
    CealSourceAttribute_VelocityY,
    CealSourceAttribute_VelocityZ,

    CealSourceAttribute_Filter_Frequency,
    CealSourceAttribute_Filter_OneOverQ,

    CealSourceAttribute_MaxEnum
};

enum CealListenerAttribute_ : uint32_t
{
    CealListenerAttribute_OrientFrontX = 0,
    CealListenerAttribute_OrientFrontY,
    CealListenerAttribute_OrientFrontZ,
    CealListenerAttribute_OrientTopX,
    CealListenerAttribute_OrientTopY,
    CealListenerAttribute_OrientTopZ,
    CealListenerAttribute_PositionX,
    CealListenerAttribute_PositionY,
    CealListenerAttribute_PositionZ,
    CealListenerAttribute_VelocityX,
    CealListenerAttribute_VelocityY,
    CealListenerAttribute_VelocityZ,

    CealListenerAttribute_3DCone_InnerAngle,    // inner cone angle in radians, must be within [0.0f, X3DAUDIO_2PI]
    CealListenerAttribute_3DCone_OuterAngle,    // outer cone angle in radians, must be within [InnerAngle, X3DAUDIO_2PI]
    CealListenerAttribute_3DCone_InnerVolume,   // volume level scaler on/within inner cone, used only for matrix calculations, must be within [0.0f, 2.0f] when used
    CealListenerAttribute_3DCone_OuterVolume,   // volume level scaler on/beyond outer cone, used only for matrix calculations, must be within [0.0f, 2.0f] when used    
    CealListenerAttribute_3DCone_InnerLPF,      // LPF (both direct and reverb paths) coefficient subtrahend on/within inner cone, used only for LPF (both direct and reverb paths) calculations, must be within [0.0f, 1.0f] when used    
    CealListenerAttribute_3DCone_OuterLPF,      // LPF (both direct and reverb paths) coefficient subtrahend on/beyond outer cone, used only for LPF (both direct and reverb paths) calculations, must be within [0.0f, 1.0f] when used    
    CealListenerAttribute_3DCone_InnerReverb,   // reverb send level scaler on/within inner cone, used only for reverb calculations, must be within [0.0f, 2.0f] when used    
    CealListenerAttribute_3DCone_OuterReverb,   // reverb send level scaler on/beyond outer cone, used only for reverb calculations, must be within [0.0f, 2.0f] when used

    CealListenerAttribute_MaxEnum,
};

// Flags and attributes
typedef uint32_t CealContextFlags;              // => CealContextFlags_
typedef uint32_t CealSourceAttribute;           // => CealSourceAttribute_ 
typedef uint32_t CealResult;                    // => CealResult_
typedef uint32_t CealListenerAttribute;         // => CealListenerAttribute_

// Allocation function
typedef void* (*CealAllocationCallback)(void* ptr, size_t oldsize, size_t newsize);

/**
 * @brief Holds info about Wav audio
 * @see http://soundfile.sapp.org/doc/WaveFormat/
 */
struct CealAudioFile_Wav
{
    uint32_t ChunkSize;

    uint32_t FmtSize; // 16 for PCM
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;

    // if not PCM
    uint16_t ExtraParamSize;

    // Data
    uint32_t DataSize;
    uint32_t DataOffset;
    uint8_t* Data;

    CealAudioFile_Wav()
    {
        memset(this, 0, sizeof(CealAudioFile_Wav));
    }
};

// Forward declarations
typedef struct CealSource_T* CealSource;
typedef struct CealBuffer_T* CealBuffer;
