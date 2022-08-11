#pragma once

#include <stdint.h>
#include <vcruntime_string.h>

#define BIT(x) (1 << x)

namespace Ceal {

	typedef struct Buffer_T {
		uint32_t ID;

		operator uint32_t() const {
			return ID;
		}

	} Buffer_T;

	typedef struct Source_T {
		uint32_t ID;

		operator uint32_t() const {
			return ID;
		}

	} Source_T;

	typedef struct Group_T {
		uint32_t ID;

		operator uint32_t() const {
			return ID;
		}
	} Group_T;

	enum CealResult : uint32_t {
		CealSuccess = 0,
		CealFailed,
		CealFileNotFound,
		CealInvalidFormat, // TODO(Urb): Expand this
		CealInvalidValue,
	};
}