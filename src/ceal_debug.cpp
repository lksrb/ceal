#include "ceal_debug.h"

#include <string>
#include <chrono>
#include <iostream>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// =============================================================================
//                             Measurement Utilities 
// =============================================================================
#ifdef CEAL_DEBUG_UTILS
CealTimer::CealTimer(const char* name, bool output /*= true*/)
    : m_Name(name), m_Output(output)
{
    m_Start = std::chrono::high_resolution_clock::now();
}

float CealTimer::ElapsedMs()
{
    m_End = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> duration = m_End - m_Start;

    return duration.count() * 1000.0f;
}

CealTimer::~CealTimer()
{
    float elapsedms = ElapsedMs();

    if (m_Output == false)
        return;

    std::cout << m_Name << " took " << elapsedms << " ms\n";
}
#endif
// =============================================================================
//                             Debug Utilities 
// =============================================================================

void ceal_debug_init()
{
    //_CrtSetBreakAlloc(264);

    // Checking memory leaks
#ifdef CEAL_DEBUG_UTILS
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void ceal_debug_check_memory_leaks()
{
    //_CrtDumpMemoryLeaks();
}

