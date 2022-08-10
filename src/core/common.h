#pragma once

#include <stdint.h>
#include <vcruntime_string.h>

#define BIT(x) (1 << x)

namespace ceal {

	typedef uint32_t Buffer_T;
	typedef uint32_t Source_T;

	enum CealResult : uint32_t {
		CealSuccess = 0,
		CealFailed,
		CealFileNotFound,
		CealInvalidFormat, // TODO(Urb): Expand this
		CealInvalidValue,
	};
}