#include <chip8cpp_app/app.hpp>

#include <chip8cpp/chip8cpp.hpp>

#include <cassert>
#include <iostream>

namespace
{
    // __  __  __  __
    // |1 ||2 ||3 ||C |
    // |4 ||5 ||6 ||D |
    // |7 ||8 ||9 ||E |
    // |A ||0 ||B ||F |
    // __  __  __  __
    // Map to
    // __  __  __  __
    // |1 ||2 ||3 ||4 |
    //   |Q ||W ||E ||R |
    //     |A ||S ||D ||F |
    //       |Z ||X ||C ||V |
    //       __  __  __  __
    SDL_Keycode getSDLKeyCode(chip8cpp::KeyCode keyCode)
    {
        switch (keyCode)
        {
            case chip8cpp::KeyCode::eNum1:
                return SDLK_1;
            case chip8cpp::KeyCode::eNum2:
                return SDLK_2;
            case chip8cpp::KeyCode::eNum3:
                return SDLK_3;
            case chip8cpp::KeyCode::eC:
                return SDLK_4;
            case chip8cpp::KeyCode::eNum4:
                return SDLK_q;
            case chip8cpp::KeyCode::eNum5:
                return SDLK_w;
            case chip8cpp::KeyCode::eNum6:
                return SDLK_e;
            case chip8cpp::KeyCode::eD:
                return SDLK_r;
            case chip8cpp::KeyCode::eNum7:
                return SDLK_a;
            case chip8cpp::KeyCode::eNum8:
                return SDLK_s;
            case chip8cpp::KeyCode::eNum9:
                return SDLK_d;
            case chip8cpp::KeyCode::eE:
                return SDLK_f;
            case chip8cpp::KeyCode::eA:
                return SDLK_z;
            case chip8cpp::KeyCode::eNum0:
                return SDLK_x;
            case chip8cpp::KeyCode::eB:
                return SDLK_c;
            case chip8cpp::KeyCode::eF:
                return SDLK_v;
            default:
                assert(0);
        }

        return SDLK_UNKNOWN; // Should never reach here
    }
} // namespace

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

            // Set key states based on user input
            setKeyStates();
        }
    }

    void App::draw()
    {
        assert(m_Renderer);
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

    void App::setKeyStates()
    {
        // Set key states based on user input
        const uint8_t* state = SDL_GetKeyboardState(nullptr);
        for (size_t i = 0; i < chip8cpp::constants::KeyCount; ++i)
        {
            chip8cpp::KeyCode keyCode   = static_cast<chip8cpp::KeyCode>(i);
            bool              isPressed = state[SDL_GetScancodeFromKey(getSDLKeyCode(keyCode))] != 0;
            m_Chip8.setKeyState(keyCode, isPressed);
        }
    }
} // namespace chip8cpp_app