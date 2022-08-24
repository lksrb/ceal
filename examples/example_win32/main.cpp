/*
 *
 * CEAL Windows example. 
 * This file serves purpose as a showcase for my library. Not the best implementation in the world but 
 * 
 **/

// =================================> Backend <==================================== //

// #include "backends/ceal_win32_xaudio2.cpp" Typically you would put this into arbitrary .cpp file in your project to compile.
#include "ceal.h"
#include "ceal_loaders.h"

// ================================================================================ //
 
// Ceal-ImGui showcase window
#include "ceal_window.h"

#include <iostream>

// Convenient macros for debugging
#define ASSERT(condition, msg) if((condition) == CEAL::Result_Failed) { std::cerr << msg << "\n"; __debugbreak(); }

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

static CEAL::Group_T s_Group;

static CEAL::Source_T s_SongSourceMono;
static CEAL::Buffer_T s_SongBufferMono;

static CEAL::Source_T s_CheersSource;
static CEAL::Buffer_T s_CheersBuffer;

static CEAL::Source_T s_SongSourceStereo;
static CEAL::Buffer_T s_SongBufferStereo;

int main()
{
    // Creating audio context
    CEAL::ContextFlags contextFlags = CEAL::ContextFlags_None;
    CEAL::CreateContext(contextFlags); // TODO(Urby): Figure out what next

    // Group is essential, you need atleast one
    CEAL::CreateGroup(&s_Group);

    // Mono song
    CEAL::AudioFile_Wav songAudioFile;
    {
        ASSERT(CEAL::LoadAudioFile_Wav("../assets/example_song_stereo.wav", &songAudioFile), "Failed to load file!");
        CEAL::CreateBuffer(&s_SongBufferMono, &songAudioFile);
        CEAL::CreateSource(&s_SongSourceMono, &songAudioFile, &s_Group);
    }

    CEAL::SetSourceFloat(&s_SongSourceMono, CEAL::SourceAttribute_Volume, 0.05f);

    // Cheers
    CEAL::AudioFile_Wav cheersAudioFile;
    {
        ASSERT(CEAL::LoadAudioFile_Wav("../assets/cheers.wav", &cheersAudioFile), "Failed to load file!");
        CEAL::CreateBuffer(&s_CheersBuffer, &cheersAudioFile);
        CEAL::CreateSource(&s_CheersSource, &cheersAudioFile, &s_Group);
    }

    // Stereo song
    CEAL::AudioFile_Wav songStereoAudioFile;
    {
        ASSERT(CEAL::LoadAudioFile_Wav("../assets/example_song_stereo.wav", &songStereoAudioFile), "Failed to load file!");
        CEAL::CreateBuffer(&s_SongBufferStereo, &songStereoAudioFile);
        CEAL::CreateSource(&s_SongSourceStereo, &songStereoAudioFile, &s_Group);
    }

    // Ceal demo window
    CEAL::Window::InitWindow();
    CEAL::Window::RegisterImGuiCallback(CealDemoDraw);
    // Window loop
    CEAL::Window::Run();

    // Do not forget to release memory.
    CEAL::DestroyContext();
    return 0;
}

static void CealDemoDraw()
{
    ImGui::Begin("Ceal Context Flags");
    static bool sound3D = false;
    if (ImGui::Checkbox("3D sound", &sound3D))
    {
        if(sound3D)
            CEAL::SetContextFlags(CEAL::ContextFlags_Enable3D);
        else
            CEAL::UnsetContextFlags(CEAL::ContextFlags_Enable3D);
            
    }

    ImGui::End();

    ImGui::Begin("Listener");
    if (ImGui::DragFloat3("Position", s_ListenerConfig.Position, 0.1f))
    {
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_PositionX, s_ListenerConfig.Position[0]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_PositionY, s_ListenerConfig.Position[1]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_PositionZ, s_ListenerConfig.Position[2]);
    }

    if (ImGui::DragFloat3("OrientFront", s_ListenerConfig.OrientFront, 0.1f))
    {
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientFrontX, s_ListenerConfig.OrientFront[0]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientFrontY, s_ListenerConfig.OrientFront[1]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientFrontZ, s_ListenerConfig.OrientFront[2]);
    }

    if (ImGui::DragFloat3("OrientTop", s_ListenerConfig.OrientTop, 0.1f))
    {
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientTopX, s_ListenerConfig.OrientTop[0]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientTopY, s_ListenerConfig.OrientTop[1]);
        CEAL::SetListenerFloat(CEAL::ListenerAttribute_OrientTopZ, s_ListenerConfig.OrientTop[2]);
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
        CEAL::SubmitBuffer(&s_SongSourceMono, &s_SongBufferMono);
        CEAL::Play(&s_SongSourceMono);
    }

    if (ImGui::Button("Stream audio!", { 50, 50 }))
    {
        CEAL::CreateStream(&s_SongSourceMono, "../assets/example_streaming.wav");
        CEAL::PlayStream(&s_SongSourceMono);
    }

    if (ImGui::DragFloat("Volume", &s_Gain, 0.01f, 0.0f, 1.0f))
    {
        CEAL::SetSourceFloat(&s_SongSourceMono, CEAL::SourceAttribute_Volume, s_Gain);
    }

    if (ImGui::DragFloat("Pitch", &s_Pitch, 0.01f, 0.0f, 2.0f))
    {
        CEAL::SetSourceFloat(&s_SongSourceMono, CEAL::SourceAttribute_Pitch, s_Pitch);
    }

    ImGui::End();

    // CEAL Perfomance

    ImGui::Begin("Performance");
    //ImGui::Text("Update: %.3f ms", g_CealStats.UpdateFuncMs);
    ImGui::End();

    ASSERT(CEAL::Update(), "Update failed!");
}
