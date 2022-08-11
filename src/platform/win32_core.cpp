// Windows API buffer
#if defined _WIN32 || defined _WIN64

#include "win32_core.h"
#include "../core/loaders.h"

#include <iostream>
#include <math.h>
#include <chrono>

#define INPUT_CHANNELS 1
#define OUTPUT_CHANNELS 8
#define CEAL_X3DAUDIO 0

#define CEAL_X3DAUDIO_CALCULATE X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB

namespace Ceal {

	namespace utils {

		static float GetElapsedTime() {
			using namespace std::chrono;
			// Can't remember off the top of my head, but this is initialized with
			// the start of the program rather than first call, right?
			static high_resolution_clock::time_point start = high_resolution_clock::now();

			return duration_cast<duration<float>>(start - high_resolution_clock::now()).count();
		}
	}

	extern CealContext* g_CealContext = nullptr;

	namespace Internal {
		/**
		 * More on https://docs.microsoft.com/en-us/windows/win32/xaudio2/debugging-facilities
		 */
		static void SetupDebuggingFacilities()
		{
			XAUDIO2_DEBUG_CONFIGURATION	debugConf{};
			debugConf.LogFunctionName = true;

			// XAUDIO2_LOG_WARNINGS also enables XAUDIO2_LOG_ERRORS
			debugConf.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
			g_CealContext->XInstance->SetDebugConfiguration(&debugConf);

			// Perfomance data
			XAUDIO2_PERFORMANCE_DATA performanceData;
			g_CealContext->XInstance->GetPerformanceData(&performanceData);
		}

		static FLOAT32 s_Matrix[8]{};

		/**
		 * Fuckin hell
		 */
		static void CalculateVolume(const Source_T sourceId)
		{
#if CEAL_X3DAUDIO
			// Query for master input channels
			XAUDIO2_VOICE_DETAILS masterDetails;
			g_CealContext->XMasterVoice->GetVoiceDetails(&masterDetails);
			// Query for source input channels
			XAUDIO2_VOICE_DETAILS sourceDetails;
			g_CealContext->XSourceVoiceMap[sourceId]->GetVoiceDetails(&sourceDetails);

			// Pupulating structs
			X3DAUDIO_DSP_SETTINGS dspSettings{};

			/*s_Matrix[0] = 0.0f;
			s_Matrix[1] = 1.0f;*/

			//matrix = new FLOAT32[8];

			std::cout << "Listener(x, y, z): (" << g_CealContext->GListener.Position.x << ", " << g_CealContext->GListener.Position.y << ", " << g_CealContext->GListener.Position.z << ")";

			dspSettings.SrcChannelCount = INPUT_CHANNELS;
			dspSettings.DstChannelCount = masterDetails.InputChannels;
			dspSettings.pMatrixCoefficients = s_Matrix;

			// For loop all the sources.. maybe?

			// Calculating
			X3DAudioCalculate(g_CealContext->X3DInstance, &g_CealContext->GListener, &g_CealContext->GAudioSourceMap[sourceId], CEAL_X3DAUDIO_CALCULATE, &dspSettings);

			// Applying matrix
			g_CealContext->XSourceVoiceMap[sourceId]->SetFrequencyRatio(dspSettings.DopplerFactor);
			g_CealContext->XSourceVoiceMap[sourceId]->SetOutputMatrix(g_CealContext->XMasterVoice, 1, masterDetails.InputChannels, dspSettings.pMatrixCoefficients);

			// Filtering
			XAUDIO2_FILTER_PARAMETERS FilterParameters = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient), 1.0f };
			g_CealContext->XSourceVoiceMap[sourceId]->SetFilterParameters(&FilterParameters);

			//g_CealContext->XSourceVoiceMap[sourceId]->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);

#elif CEAL_X3DAUDIO
			auto pSourceVoice = &g_CealContext->XSourceVoiceMap[sourceId];

			// Pan
			DWORD dwChannelMask;
			g_CealContext->XMasterVoice->GetChannelMask(&dwChannelMask);

			float factor = 1.0f;
			float range = 1.0f;

			float pan = range * std::sin(utils::GetElapsedTime() * factor);

			float outputMatrix[8]; // 3 * 3 matrix
			for (int i = 0; i < 8; i++)
			{
				outputMatrix[i] = 0;
				// pan of -1.0 indicates all left speaker,
				// 1.0 is all right speaker, 0.0 is split between left and right
				float left = 0.5f - pan / 2;
				float right = 0.5f + pan / 2;

				// Swithing
				switch (dwChannelMask)
				{
				case SPEAKER_MONO:
					outputMatrix[0] = 1.0;
					break;
				case SPEAKER_STEREO:
				case SPEAKER_2POINT1:
				case SPEAKER_SURROUND:
					outputMatrix[0] = left;
					outputMatrix[1] = right;
					break;
				case SPEAKER_QUAD:
					outputMatrix[0] = outputMatrix[2] = left;
					outputMatrix[1] = outputMatrix[3] = right;
					break;
				case SPEAKER_4POINT1:
					outputMatrix[0] = outputMatrix[3] = left;
					outputMatrix[1] = outputMatrix[4] = right;
					break;
				case SPEAKER_5POINT1:
				case SPEAKER_7POINT1:
				case SPEAKER_5POINT1_SURROUND:
					outputMatrix[0] = outputMatrix[4] = left;
					outputMatrix[1] = outputMatrix[5] = right;
					break;
				case SPEAKER_7POINT1_SURROUND:
					outputMatrix[0] = outputMatrix[4] = outputMatrix[6] = left;
					outputMatrix[1] = outputMatrix[5] = outputMatrix[7] = right;
					break;
				}
			}

			// Assuming pVoice sends to pMasteringVoice

			XAUDIO2_VOICE_DETAILS voiceDetails;
			(*pSourceVoice)->GetVoiceDetails(&voiceDetails);

			XAUDIO2_VOICE_DETAILS masterVoiceDetails;
			g_CealContext->XMasterVoice->GetVoiceDetails(&masterVoiceDetails);

			(*pSourceVoice)->SetOutputMatrix(g_CealContext->XMasterVoice, voiceDetails.InputChannels, masterVoiceDetails.InputChannels, outputMatrix);
#endif
		}
	}

	CealResult CreateContext()
	{
		g_CealContext = new CealContext;

		// Create COM
		CEAL_CHECK_XA2(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

		// Create audio engine instance 
		CEAL_CHECK_XA2(XAudio2Create(&g_CealContext->XInstance, XAUDIO2_DEBUG_ENGINE, XAUDIO2_DEFAULT_PROCESSOR));

		// Setup debug and log info for EventViewer by Windows
		Internal::SetupDebuggingFacilities();

		// Create mastering voice
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateMasteringVoice(&g_CealContext->XMasterVoice, 2));

		// Create Submit voice
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSubmixVoice(&g_CealContext->XSubmitVoice, 1, 44100, 0, 0, 0, 0));

		DWORD dwChannelMask;
		CEAL_CHECK_XA2(g_CealContext->XMasterVoice->GetChannelMask(&dwChannelMask));

		// Create 3d audio
		CEAL_CHECK_XA2(X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, g_CealContext->X3DInstance));

		return CealResult::CealSuccess;
	}

	CealResult DestroyContext()
	{
		// Release buffers
		for (const auto& [id, buffer] : g_CealContext->XBufferMap) {
			delete buffer.pAudioData;
		}

		// Release source voices
		for (const auto& [id, sourceVoice] : g_CealContext->XSourceVoiceMap) {
			sourceVoice->Stop();
			sourceVoice->DestroyVoice();
		}

		g_CealContext->XMasterVoice->DestroyVoice();
		g_CealContext->XInstance->Release();

		delete g_CealContext;

		return CealResult::CealSuccess;
	}

	CealResult CreateSource(Source_T* sourceId, const AudioFile_Wav* audioFile)
	{
		sourceId->ID = g_CealContext->GlobalIncrementId++;

		WAVEFORMATEX wfx = { 0 };
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = audioFile->NumChannels;
		wfx.nSamplesPerSec = audioFile->SampleRate;
		wfx.wBitsPerSample = audioFile->BitsPerSample;
		wfx.cbSize = audioFile->ExtraParamSize;
		wfx.nBlockAlign = audioFile->BlockAlign;
		wfx.nAvgBytesPerSec = audioFile->ByteRate;

		// Create Source Voice
		auto pSourceVoice = &g_CealContext->XSourceVoiceMap[*sourceId];
		CEAL_CHECK_XA2(g_CealContext->XInstance->CreateSourceVoice(pSourceVoice, &wfx));

		return CealResult::CealSuccess;
	}

	// Submits buffer to a source queue.
	CealResult SubmitBuffer(const Source_T sourceId, const Buffer_T bufferId)
	{
		const auto& buffer = g_CealContext->XBufferMap[bufferId];
		const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
		// Submitting buffer
		CEAL_CHECK_XA2(pSource->SubmitSourceBuffer(&buffer));

		return CealResult::CealSuccess;
	}

	CealResult Play(const Source_T sourceId)
	{
		const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
		// Playing audio
		CEAL_CHECK_XA2(pSource->Start(0, XAUDIO2_COMMIT_NOW));

		return CealResult::CealSuccess;
	}

	CealResult SetVolume(const Source_T sourceId, float volume)
	{
		if (volume >= 0.0f) {
			const auto& pSource = g_CealContext->XSourceVoiceMap[sourceId];
			CEAL_CHECK_XA2(pSource->SetVolume(volume));
			return CealResult::CealSuccess;
		}

		return CealResult::CealInvalidValue;
	}

	// Listener/Source relationship used in game

	// Audio Listener
	void ConfigureAudioListener(const AudioListenerConfigV2* config)
	{
		const AudioListenerConfigV2& cfg = *config;

		static const X3DAUDIO_CONE listenerDirectionalCone = { X3DAUDIO_PI * 5.0f / 6.0f, X3DAUDIO_PI * 11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

		g_CealContext->GListener = {};
		g_CealContext->GListener.Position = X3DAUDIO_VECTOR(cfg.Position);
		g_CealContext->GListener.Velocity = X3DAUDIO_VECTOR(cfg.Velocity);
		g_CealContext->GListener.OrientFront = X3DAUDIO_VECTOR(cfg.OrientFront);
		g_CealContext->GListener.OrientTop = X3DAUDIO_VECTOR(cfg.OrientTop);
		g_CealContext->GListener.pCone = (X3DAUDIO_CONE*)&listenerDirectionalCone;
	}

	// Audio Source
	void ConfigureAudioSource(const Source_T sourceid, const AudioSourceConfig* config)
	{
		const AudioSourceConfig& cfg = *config;

		static const X3DAUDIO_CONE listenerDirectionalCone2 = { X3DAUDIO_PI * 5.0f / 6.0f, X3DAUDIO_PI * 11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

		auto& audioSource = g_CealContext->GAudioSourceMap[sourceid];

		audioSource = {};
		audioSource.CurveDistanceScaler = FLT_MIN;
		audioSource.ChannelCount = 1;
		audioSource.Position = X3DAUDIO_VECTOR(cfg.Position);
		audioSource.Velocity = X3DAUDIO_VECTOR(cfg.Velocity);
		audioSource.pCone = (X3DAUDIO_CONE*)&listenerDirectionalCone2;
		audioSource.pCone = (X3DAUDIO_CONE*)&listenerDirectionalCone2;
		audioSource.ChannelRadius = 1.0f;
		audioSource.InnerRadius = 2.0f;
		audioSource.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
	}

	void Update(const Source_T sourceId) {
		//Internal::CalculateVolume(sourceId);
	}
}

#endif