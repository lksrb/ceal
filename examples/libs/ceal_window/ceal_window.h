#pragma once

#include "imgui.h"

namespace Ceal::Window {
    void InitWindow();
    void Run();
    void RegisterImGuiCallback(void (*function)());
}
