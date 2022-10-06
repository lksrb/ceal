#pragma once

#include <chrono>

// =============================================================================
//                             Measurement Utilities 
// =============================================================================
//#define CEAL_DEBUG_UTILS
#ifdef CEAL_DEBUG_UTILS

class CealTimer
{
public:
    CealTimer(const char* name, bool output = true);
    ~CealTimer();

    float ElapsedMs();

    const char* GetName() const { return m_Name; }

    static inline CealTimer Create(const char* name) { return { name }; }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
    const char* m_Name;
    bool m_Output;
};

#   define CEAL_CONCAT(a, b) a ## b 
#   define CEAL_MEASURE_FUNCTION() CealTimer CEAL_CONCAT(timer, __FUNCTION__)("[" __FUNCTION__ "]")
#   define CEAL_MEASURE_SCOPE(name) CealTimer CEAL_CONCAT(timer, __LINE__)("[" name "]")
#else
#   define CEAL_MEASURE_FUNCTION()
#   define CEAL_MEASURE_SCOPE(name)
#endif

// =============================================================================
//                             Debug Utilities 
// =============================================================================

void ceal_debug_init();
void ceal_debug_check_memory_leaks();
