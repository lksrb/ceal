/*
 *
 * CEAL Windows example. 
 * This file serves purpose as a showcase for my library. Not the best implementation in the world but 
 * 
 **/

// =================================> Backend <====================================

#include "backends/ceal_win32_xaudio2.h"

#include "ceal.h"
#include "ceal_loaders.h"

// ================================================================================
 
// Ceal-ImGui showcase window
#include "ceal_window.h"

#include <iostream>

// Memory leak d
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Convenient macros for debugging
#define ASSERT(condition, msg) if((condition) == Ceal::Result_Failed) { std::cerr << msg << "\n"; __debugbreak(); }

// Definitions
void CealDemoDraw();

struct ListenerAttributes3D
{
    float Position[3] =     { 0.0f, 0.0f, 0.0f };
    float Velocity[3] =     { 0.0f, 0.0f, 0.0f };
    float OrientFront[3] =  { 1.0f, 0.0f, 0.0f };
    float OrientTop[3] =    { 0.0f, 0.0f, 1.0f };
};

static ListenerAttributes3D s_ListenerConfig;

static Ceal::Group_T s_Group;

static Ceal::Source_T s_SongSourceMono;
static Ceal::Buffer_T s_SongBufferMono;

static Ceal::Source_T s_CheersSource;
static Ceal::Buffer_T s_CheersBuffer;

static Ceal::Source_T s_SongSourceStereo;
static Ceal::Buffer_T s_SongBufferStereo;

int main()
{
    // Checking memory leaks, you can ignore this part
    {
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }

    // Creating audio context
    Ceal::ContextFlags contextFlags = Ceal::ContextFlags_None;
    Ceal::CreateContext(contextFlags);

    // Group is essential, you need atleast one
    Ceal::CreateGroup(&s_Group);

    // Mono song
    Ceal::AudioFile_Wav songAudioFile;
    {
        ASSERT(Ceal::LoadAudioFile_Wav("../assets/example_song_stereo.wav", &songAudioFile), "Failed to load file!");
        Ceal::CreateBuffer(&s_SongBufferMono, &songAudioFile);
        Ceal::CreateSource(&s_SongSourceMono, &songAudioFile, &s_Group);
    }

    Ceal::SetSourceFloat(&s_SongSourceMono, Ceal::SourceAttribute_Volume, 0.05f);

    // Cheers
    Ceal::AudioFile_Wav cheersAudioFile;
    {
        ASSERT(Ceal::LoadAudioFile_Wav("../assets/cheers.wav", &cheersAudioFile), "Failed to load file!");
        Ceal::CreateBuffer(&s_CheersBuffer, &cheersAudioFile);
        Ceal::CreateSource(&s_CheersSource, &cheersAudioFile, &s_Group);
    }

    // Stereo song
    Ceal::AudioFile_Wav songStereoAudioFile;
    {
        ASSERT(Ceal::LoadAudioFile_Wav("../assets/example_song_stereo.wav", &songStereoAudioFile), "Failed to load file!");
        Ceal::CreateBuffer(&s_SongBufferStereo, &songStereoAudioFile);
        Ceal::CreateSource(&s_SongSourceStereo, &songStereoAudioFile, &s_Group);
    }

    // Ceal demo window
    Ceal::Window::InitWindow();
    Ceal::Window::RegisterImGuiCallback(CealDemoDraw);
    // Window loop
    Ceal::Window::Run();

    // Do not forget to release memory.
    Ceal::DestroyContext();
    return 0;
}

static void CealDemoDraw()
{
    ImGui::Begin("Ceal Context Flags");
    static bool sound3D = false;
    if (ImGui::Checkbox("3D sound", &sound3D))
    {
        if(sound3D)
            Ceal::SetContextFlags(Ceal::ContextFlags_Enable3D);
        else
            Ceal::UnsetContextFlags(Ceal::ContextFlags_Enable3D);
            
    }

    ImGui::End();

    ImGui::Begin("Listener");
    if (ImGui::DragFloat3("Position", s_ListenerConfig.Position, 0.1f))
    {
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_PositionX, s_ListenerConfig.Position[0]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_PositionY, s_ListenerConfig.Position[1]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_PositionZ, s_ListenerConfig.Position[2]);
    }

    if (ImGui::DragFloat3("OrientFront", s_ListenerConfig.OrientFront, 0.1f))
    {
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientFrontX, s_ListenerConfig.OrientFront[0]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientFrontY, s_ListenerConfig.OrientFront[1]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientFrontZ, s_ListenerConfig.OrientFront[2]);
    }

    if (ImGui::DragFloat3("OrientTop", s_ListenerConfig.OrientTop, 0.1f))
    {
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientTopX, s_ListenerConfig.OrientTop[0]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientTopY, s_ListenerConfig.OrientTop[1]);
        Ceal::SetListenerFloat(Ceal::ListenerAttribute_OrientTopZ, s_ListenerConfig.OrientTop[2]);
    }
    ImGui::End();

    ImGui::Begin("Song Source #1");

    //ImGui::DragFloat4("Matrix", g_CealStats.Matrix4x1, 0.1f);
    //ImGui::DragFloat("Doppler Factor", &g_CealStats.DopplerFactor, 0.1f);
    //ImGui::DragFloat("LPFDirectCoefficient", &g_CealStats.LPFDirectCoefficient, 0.1f);
    //ImGui::Separator();

    static float s_Gain = 0.01f;
    static float s_Pitch = 1.0f;

    if (ImGui::Button("Play audio!", { 50, 50 }))
    {
        Ceal::SubmitBuffer(&s_SongSourceMono, &s_SongBufferMono);
        Ceal::Play(&s_SongSourceMono);
    }

    if (ImGui::Button("Stream audio!", { 50, 50 }))
    {
        Ceal::CreateStream(&s_SongSourceMono, "../assets/example_streaming.wav");
        Ceal::PlayStream(&s_SongSourceMono);
    }

    if (ImGui::DragFloat("Volume", &s_Gain, 0.01f, 0.0f, 1.0f))
    {
        Ceal::SetSourceFloat(&s_SongSourceMono, Ceal::SourceAttribute_Volume, s_Gain);
    }

    if (ImGui::DragFloat("Pitch", &s_Pitch, 0.01f, 0.0f, 2.0f))
    {
        Ceal::SetSourceFloat(&s_SongSourceMono, Ceal::SourceAttribute_Pitch, s_Pitch);
    }

    ImGui::End();

    // CEAL Perfomance

    ImGui::Begin("Performance");
    //ImGui::Text("Update: %.3f ms", g_CealStats.UpdateFuncMs);
    ImGui::End();

    ASSERT(Ceal::Update(), "Update failed!");
}
