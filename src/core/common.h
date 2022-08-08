#pragma once

#include <stdint.h>
#include <vcruntime_string.h>

#define BIT(x) (1 << x)

namespace ceal {

	typedef uint32_t AudioBufferID;

	enum CealResult : uint32_t {
		CealSuccess = 0,
		CealFailed,
		CealFileNotFound,
		CealInvalidFormat, // TODO(Urb): Expand this
	};
}