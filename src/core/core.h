#pragma once

#include "common.h"

// Client API code
namespace ceal {

	// Creates platform specific context
	CealResult CreateContext();

	// Destroys context
	CealResult DestroyContext();
}