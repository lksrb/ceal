// =============================================================================
//                             Debug Utilities 
// =============================================================================
#pragma once

#include <chrono>

namespace CEAL 
{
    class ScopedTimer
    {
    public:
        ScopedTimer(const char* name, bool output = true);
        ~ScopedTimer();

        float ElapsedMs();

        const char* GetName() const { return m_Name; }

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
        const char* m_Name;
        bool m_Output;
    };

    namespace Debug {
        void Inititialize();
    }
}
