/*
 * CEAL Windows example. 
 * This file serves purpose as a showcase for my library. 
 **/

// =================================> Backend <==================================== 
// #include "backends/ceal_win32_xaudio2.cpp" NOTE: Typically you would put this into arbitrary .cpp file in your project to compile.
#include "ceal.h"
#include "ceal_loaders.h"
#include "ceal_debug.h"
#include "backends/ceal_win32_xaudio2.h"
#include <new>
// ================================================================================ 
 
// Ceal-ImGui showcase window
#include "ceal_window.h"

#include <assert.h>
#include <iostream>

// Conveniente macros for debugging
#define ASSERT(condition, msg) if((condition) == CealResult_Failed) { std::cerr << msg << "\n"; assert(false); }

// Declarations
void CealDemoDraw();
void ShowMenuBar();

struct ListenerAttributes3D
{
    float Position[3] =     { 0.0f, 0.0f, 0.0f };
    float Velocity[3] =     { 0.0f, 0.0f, 0.0f };
    float OrientFront[3] =  { 1.0f, 0.0f, 0.0f };
    float OrientTop[3] =    { 0.0f, 0.0f, 1.0f };
};

static ListenerAttributes3D s_ListenerConfig;

// Audio sources
static CealSource s_SongSourceMono;
static CealSource s_SongSourceStereo;
static CealSource s_CheersSource;

// Audio buffers
static CealBuffer s_SongBufferMono;
static CealBuffer s_SongBufferStereo;
static CealBuffer s_CheersBuffer;

int main()
{
    // Creating audio context
    ceal_context_create();
    ceal_backend_win32_xaudio2_init(); 

    // Mono song
    CealAudioFile_Wav songAudioFile;
    {
        ASSERT(ceal_audio_file_wav_load("../assets/example_song_stereo.wav", &songAudioFile), "Failed to load file!");
        ceal_buffer_create(&s_SongBufferMono, &songAudioFile);
        ceal_source_create(&s_SongSourceMono, &songAudioFile);
    }

    ceal_source_set_float(s_SongSourceMono, CealSourceAttribute_Volume, 0.05f);

    // Cheers
    CealAudioFile_Wav cheersAudioFile;
    {
        ASSERT(ceal_audio_file_wav_load("../assets/cheers.wav", &cheersAudioFile), "Failed to load file!");
        ceal_buffer_create(&s_CheersBuffer, &cheersAudioFile);
        ceal_source_create(&s_CheersSource, &cheersAudioFile);
    }

    // Stereo song
    CealAudioFile_Wav songStereoAudioFile;
    {
        ASSERT(ceal_audio_file_wav_load("../assets/example_song_stereo.wav", &songStereoAudioFile), "Failed to load file!");
        ceal_buffer_create(&s_SongBufferStereo, &songStereoAudioFile);
        ceal_source_create(&s_SongSourceStereo, &songStereoAudioFile);
    }

    // Ceal demo window using ImGui
    {
        ceal_window_init();
        ceal_window_register_callback_imgui(CealDemoDraw);
        // Window loop
        ceal_window_run();
    }

    // Release memory
    ceal_backend_win32_xaudio2_shutdown();
    ceal_context_destroy();

    // NOTE: CEAL do not release audiofile's data allocated from LoadAudioFile_Wav(...) to keep loading files and actual audio engine separate.
    // It is recommended to release buffers after everything is shutdown.
    ceal_audio_file_wav_free(&songAudioFile);
    ceal_audio_file_wav_free(&cheersAudioFile);
    ceal_audio_file_wav_free(&songStereoAudioFile);

    return 0;
}

static void CealDemoDraw()
{
    ImGui::Begin("Ceal Context Flags");
    static bool sound3D = false;
    if (ImGui::Checkbox("3D sound", &sound3D))
    {
        if(sound3D)
            ceal_context_set_flags(CealContextFlags_Enable3D);
        else
            ceal_context_unset_flags(CealContextFlags_Enable3D);
            
    }

    ImGui::End();

    ImGui::Begin("Listener");
    if (ImGui::DragFloat3("Position", s_ListenerConfig.Position, 0.1f))
    {
        ceal_listener_set_float(CealListenerAttribute_PositionX, s_ListenerConfig.Position[0]);
        ceal_listener_set_float(CealListenerAttribute_PositionY, s_ListenerConfig.Position[1]);
        ceal_listener_set_float(CealListenerAttribute_PositionZ, s_ListenerConfig.Position[2]);
    }

    if (ImGui::DragFloat3("OrientFront", s_ListenerConfig.OrientFront, 0.1f))
    {
        ceal_listener_set_float(CealListenerAttribute_OrientFrontX, s_ListenerConfig.OrientFront[0]);
        ceal_listener_set_float(CealListenerAttribute_OrientFrontY, s_ListenerConfig.OrientFront[1]);
        ceal_listener_set_float(CealListenerAttribute_OrientFrontZ, s_ListenerConfig.OrientFront[2]);
    }

    if (ImGui::DragFloat3("OrientTop", s_ListenerConfig.OrientTop, 0.1f))
    {
        ceal_listener_set_float(CealListenerAttribute_OrientTopX, s_ListenerConfig.OrientTop[0]);
        ceal_listener_set_float(CealListenerAttribute_OrientTopY, s_ListenerConfig.OrientTop[1]);
        ceal_listener_set_float(CealListenerAttribute_OrientTopZ, s_ListenerConfig.OrientTop[2]);
    }
    ImGui::End();

    ImGui::Begin("Song Source #1");

    static float s_Gain = 0.01f;
    static float s_Pitch = 1.0f;

    if (ImGui::Button("Play audio!", { 100, 50 }))
    {
        ceal_buffer_submit(s_SongSourceMono, s_SongBufferMono);
        ceal_source_play(s_SongSourceMono);
    }

/*
    if (ImGui::Button("Stream audio!", { 50, 50 }))
    {
        ceal_source_stream_create(s_SongSourceMono, "../assets/example_streaming.wav");
        ceal_source_stream_play(s_SongSourceMono);
    }*/

    if (ImGui::DragFloat("Volume", &s_Gain, 0.01f, 0.0f, 1.0f))
    {
        ceal_source_set_float(s_SongSourceMono, CealSourceAttribute_Volume, s_Gain);
    }

    if (ImGui::DragFloat("Pitch", &s_Pitch, 0.01f, 0.0f, 2.0f))
    {
        ceal_source_set_float(s_SongSourceMono, CealSourceAttribute_Pitch, s_Pitch);
    }

    ImGui::End();

    // Menu bar for interactive showcase
    ShowMenuBar();

    ASSERT(ceal_context_update(), "Update failed!");
}

void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New source...")) // TODO(Urby): Create interactive demo example
            {

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
