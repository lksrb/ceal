#pragma once

#include "imgui.h"

namespace CEAL::Window {
    void InitWindow();
    void Run();
    void RegisterImGuiCallback(void (*function)());
}
