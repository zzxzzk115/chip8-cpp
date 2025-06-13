#include <chip8cpp_app/app.hpp>

#include <chip8cpp/chip8cpp.hpp>

#include <iostream>

namespace chip8cpp_app
{
    bool App::init(int argc, char* argv[])
    {
        // Initialize the Chip8 interpreter with configurations
        chip8cpp::Config config {};
        config.pixelOutline = true; // Enable pixel outlines for better visibility
        m_Chip8.setConfig(config);

#ifdef DEBUG
        (void)argc;
        (void)argv;

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
        m_Window = SDL_CreateWindow("Chip8 Interpreter",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    config.pixelScale * chip8cpp::constants::Width,
                                    config.pixelScale * chip8cpp::constants::Height,
                                    SDL_WINDOW_SHOWN);

        // Create a renderer
        if (!m_Window)
        {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);

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
                draw();
            }

            // TODO: Set key states based on user input
        }
    }

    void App::draw()
    {
        const chip8cpp::Config& config = m_Chip8.getConfig();

        SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_Renderer);

        // Render the graphics buffer (m_GFX) to the SDL window
        const uint8_t* gfx = m_Chip8.getGFX();
        for (size_t y = 0; y < chip8cpp::constants::Height; ++y)
        {
            for (size_t x = 0; x < chip8cpp::constants::Width; ++x)
            {
                size_t index = x + y * chip8cpp::constants::Width;
                if (gfx[index] == 1) // Pixel is on
                {
                    const int scale = m_Chip8.getConfig().pixelScale;
                    SDL_Rect  rect  = {static_cast<int>(x * scale), static_cast<int>(y * scale), scale, scale};

                    if (!config.pixelOutline)
                    {
                        SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
                        SDL_RenderFillRect(m_Renderer, &rect);
                    }
                    else
                    {
                        // Draw outline
                        SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
                        SDL_RenderDrawRect(m_Renderer, &rect);

                        // Draw filled rectangle for the pixel
                        SDL_Rect outlineRect = {rect.x + config.pixelOutlineWidth,
                                                rect.y + config.pixelOutlineWidth,
                                                rect.w - 2 * config.pixelOutlineWidth,
                                                rect.h - 2 * config.pixelOutlineWidth};
                        SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
                        SDL_RenderFillRect(m_Renderer, &outlineRect);
                    }
                }
            }
        }

        SDL_RenderPresent(m_Renderer);
    }
} // namespace chip8cpp_app