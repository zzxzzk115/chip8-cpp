#include <chip8cpp_app/app.hpp>

#include <chip8cpp/chip8cpp.hpp>

#include <iostream>

namespace chip8cpp_app
{
    bool App::init(int argc, char* argv[])
    {
#ifdef DEBUG
        if (!m_Chip8.loadProgram("programs/2-ibm-logo.ch8"))
        {
            std::cerr << "Failed to load default program." << std::endl;
            return false;
        }
#else
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <program_file>" << std::endl;
            return false;
        }

        const std::string programFile = argv[1];
        if (!m_Chip8.loadProgram(programFile))
        {
            std::cerr << "Failed to load program: " << programFile << std::endl;
            return false;
        }
#endif

        // Initialize the SDL2
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create a window
        m_Window = SDL_CreateWindow(
            "Chip8 Interpreter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);

        return true;
    }

    void App::run()
    {
        while (true)
        {
            // Handle events
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    return; // Exit the application
                }
            }

            // Emulate one cycle of the Chip8 interpreter
            m_Chip8.emulateOneCycle();

            // If the Chip8 interpreter has a draw flag, render the graphics
            if (m_Chip8.getDrawFlag())
            {
                SDL_SetRenderDrawColor(SDL_GetRenderer(m_Window), 0, 0, 0, 255);
                SDL_RenderClear(SDL_GetRenderer(m_Window));

                // TODO: Render the graphics buffer (m_GFX) to the SDL window

                SDL_RenderPresent(SDL_GetRenderer(m_Window));
            }

            // TODO: Set key states based on user input
        }
    }
} // namespace chip8cpp_app