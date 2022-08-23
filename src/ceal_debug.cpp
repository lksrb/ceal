#include "ceal_debug.h"

#include <string>
#include <chrono>
#include <iostream>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace Ceal 
{
    ScopedTimer::ScopedTimer(const char* name, bool output /*= true*/)
        : m_Name(name), m_Output(output)
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    float ScopedTimer::ElapsedMs()
    {
        m_End = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> duration = m_End - m_Start;

        return duration.count() * 1000.0f;
    }

    ScopedTimer::~ScopedTimer()
    {
        if (m_Output == false)
            return;

        float elapsedms = ElapsedMs();

        std::cout << m_Name << " took " << elapsedms << " ms\n";
    }

    void Debug::Inititialize()
    {
        // Checking memory leaks
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }

}
