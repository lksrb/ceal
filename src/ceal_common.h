/**
 *
 * Ceal commons
 *
 */
#pragma once

#include "ceal_types.h"

#include <assert.h>
#include <new>

// MSVC only
#ifdef _MSC_VER
#   define CEAL_ASSERT(condition) if((condition) == false)  { assert(false); }
#   define CEAL_VERIFY(condition) if((condition))           { assert(false); }
#endif

extern CealAllocationCallback g_AllocationCallback;

// Memory allocators
#define CEAL_MEM_ALLOC(__object) (__object*)g_AllocationCallback(nullptr, 0, sizeof(__object))
#define CEAL_MEM_ALLOC_SIZE(__object, __size) (__object*)g_AllocationCallback(nullptr, 0, __size)
#define CEAL_MEM_ARRAY_ALLOC(__object, __length) (__object*)g_AllocationCallback(nullptr, 0, sizeof(__object) * __length)
#define CEAL_MEM_FREE(__pointer) g_AllocationCallback(__pointer, sizeof(*__pointer), 0)
#define CEAL_MEM_FREE_SIZE(__object, __pointer) g_AllocationCallback(__pointer, sizeof(__object), 0)
#define CEAL_MEM_ARRAY_FREE(__pointer, __length) g_AllocationCallback(__pointer, sizeof(*__pointer) * __length, 0)

// Memory allocators that trigger the constuctor/destructors
#define CEAL_NEW(__object) new(CEAL_MEM_ALLOC(__object))__object() 
#define CEAL_DELETE(__object, __pointer) __pointer->~__object(); CEAL_MEM_FREE(__pointer)



