#include "ceal_debug.h"

#include <string>
#include <chrono>
#include <iostream>

namespace Ceal {
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

}
