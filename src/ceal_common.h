#pragma once

#include <stdint.h> // uint32_t
#include <vcruntime_string.h> // memset, ...

namespace CEAL {
	
	struct Buffer_T 
	{
		uint32_t ID;
		inline operator uint32_t() const { return ID; }
	};
	
	struct Source_T 
	{
		uint32_t ID;
		inline operator uint32_t() const { return ID; }
	};
	
	struct Group_T
	{
		uint32_t ID;
		inline operator uint32_t() const { return ID; }
	};
	
	struct CealVector3 
	{
		float X;
		float Y;
		float Z;

		constexpr CealVector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
		constexpr CealVector3(float* arrayf) : X(arrayf[0]), Y(arrayf[1]), Z(arrayf[2]) {}
	};

	enum Result_ : uint32_t 
	{
		Result_Failed = 0,
		Result_Success = 1 << 0,
		Result_FileNotFound = 1 << 1,
		Result_InvalidFormat,
		Result_InvalidValue,
	};
	
	enum ContextFlags_ : uint32_t 
	{
		ContextFlags_None = 0,
		ContextFlags_Enable3D = 1 << 0,
		ContextFlags_Something = 1 << 1,
	};

    enum SourceAttribute_ : uint32_t 
	{
		SourceAttribute_Volume = 0,
		SourceAttribute_Pitch,

        SourceAttribute_OrientFrontX,
        SourceAttribute_OrientFrontY,
        SourceAttribute_OrientFrontZ,
        SourceAttribute_OrientTopX,
        SourceAttribute_OrientTopY,
        SourceAttribute_OrientTopZ,
        SourceAttribute_PositionX,
        SourceAttribute_PositionY,
        SourceAttribute_PositionZ,
        SourceAttribute_VelocityX,
        SourceAttribute_VelocityY,
        SourceAttribute_VelocityZ,

        SourceAttribute_MaxEnum
    };

    enum ListenerAttribute_ : uint32_t
    {
        ListenerAttribute_OrientFrontX = 0,
        ListenerAttribute_OrientFrontY,
        ListenerAttribute_OrientFrontZ,
        ListenerAttribute_OrientTopX,
        ListenerAttribute_OrientTopY,
        ListenerAttribute_OrientTopZ,
        ListenerAttribute_PositionX,
        ListenerAttribute_PositionY,
        ListenerAttribute_PositionZ,
        ListenerAttribute_VelocityX,
        ListenerAttribute_VelocityY,
        ListenerAttribute_VelocityZ,

        ListenerAttribute_MaxEnum,
    };

	// Flags and attributes
	typedef uint32_t ContextFlags; // => ContextFlags_
	typedef uint32_t SourceAttribute; // => SourceAttribute_ 
	typedef uint32_t Result; // => Result_
	typedef uint32_t ListenerAttribute; // => ListenerAttribute_

    /**
     * @brief Holds info about Wav audio
     * @see http://soundfile.sapp.org/doc/WaveFormat/
     */
    struct AudioFile_Wav
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

        AudioFile_Wav()
        {
            memset(this, 0, sizeof(AudioFile_Wav));
        }
    };
}

// =============================================================================
//					                 Macros
// =============================================================================
 
#define CEAL_INVALID_ID 0

