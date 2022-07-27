#pragma once

#include "pch.hpp"
#include "NodeEditor.hpp"

namespace Billyprints {
    class Billyprints
    {
    public:
        static void glfw_error_callback(int error, const char* description);

        Billyprints();
        int Mainloop();
    };
}
