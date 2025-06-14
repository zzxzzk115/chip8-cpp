#pragma once

#define SDL_MAIN_HANDLED // Prevents SDL from defining main() on Windows
#include <SDL.h>
#include <chip8cpp/chip8cpp.hpp>

namespace chip8cpp_app
{
    class App
    {
    public:
        App()  = default;
        ~App() = default;

        bool init(int argc, char* argv[]);
        void run();

    private:
        void draw();
        void setKeyStates();

    private:
        chip8cpp::Chip8   m_Chip8;              // Instance of the Chip8 interpreter
        SDL_Window*       m_Window {nullptr};   // SDL window for rendering
        SDL_Renderer*     m_Renderer {nullptr}; // SDL renderer for drawing
        SDL_AudioDeviceID m_AudioDeviceID {0};  // SDL audio device ID for sound output
    };
} // namespace chip8cpp_app