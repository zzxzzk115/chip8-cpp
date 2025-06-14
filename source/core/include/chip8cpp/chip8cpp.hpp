#pragma once

#include <string>

namespace chip8cpp
{
    namespace constants
    {
        constexpr uint8_t  Width               = 64;             // Width of the Chip-8 screen in pixels
        constexpr uint8_t  Height              = 32;             // Height of the Chip-8 screen in pixels
        constexpr uint16_t ProgramStartAddress = 0x200;          // Starting address for programs in Chip-8 memory
        constexpr size_t   MemorySize          = 4096;           // Total memory size for Chip-8
        constexpr size_t   StackSize           = 16;             // Size of the stack for Chip-8
        constexpr size_t   GfxSize             = Width * Height; // Size of the graphics buffer (64x32 pixels)
        constexpr size_t   FontSetSize         = 80;             // Size of the font set (5x16 pixels for 16 characters)
        constexpr size_t   RegisterCount       = 16;             // Number of registers in Chip-8
        constexpr size_t   KeyCount            = 16;             // Number of keys in Chip-8 (0-F)
    } // namespace constants

    struct Config
    {
        int  pixelScale {10};       // Scale factor for each pixel in the graphics buffer
        int  pixelOutlineWidth {1}; // Width of pixel outlines in the graphics buffer
        bool pixelOutline {false};  // Whether to draw pixel outlines in the graphics buffer

#ifdef DEBUG
        bool printAsciiGraphics {false}; // Whether to print graphics buffer as ASCII art in the console
        bool printKeyStates {true};      // Whether to print key states in the console
#endif
    };

    // https://tobiasvl.github.io/assets/images/cosmac-vip-keypad.png
    // __  __  __  __
    // |1 ||2 ||3 ||C |
    // |4 ||5 ||6 ||D |
    // |7 ||8 ||9 ||E |
    // |A ||0 ||B ||F |
    // __  __  __  __
    enum class KeyCode
    {
        eNum1 = 0,
        eNum2,
        eNum3,
        eC,
        eNum4,
        eNum5,
        eNum6,
        eD,
        eNum7,
        eNum8,
        eNum9,
        eE,
        eA,
        eNum0,
        eB,
        eF,
    };

    class Chip8
    {
    public:
        explicit Chip8(const Config& config = {});

        void          setConfig(const Config& config);
        const Config& getConfig() const;

        bool loadProgram(const std::string& fileName);

        void emulateOneCycle();

        bool isKeyPressed(KeyCode keyCode) const;
        void setKeyState(KeyCode keyCode, bool isPressed);

        bool getDrawFlag() const;

        const uint8_t* getGFX() const;

    private:
        void reset();

        uint16_t fetchOpcode();
        void     decodeAndExecuteOpcode(uint16_t opcode);
        void     updateTimers();

        void loadFontSet();

    private:
        Config m_Config {}; // Configuration settings

        uint8_t  m_V[constants::RegisterCount] {};      // Registers
        uint8_t  m_DelayTimer {0};                      // Delay timer
        uint8_t  m_SoundTimer {0};                      // Sound timer
        uint8_t  m_SP {0};                              // Stack pointer
        uint8_t  m_Keys[constants::KeyCount] {};        // Key states, 0 for up, 1 for down, KeyCode is the index
        uint8_t  m_GFX[constants::GfxSize] {};          // Graphics buffer (64x32 pixels)
        bool     m_DrawFlag {false};                    // Flag to indicate if a redraw is needed
        uint16_t m_I {0};                               // Index register
        uint16_t m_PC {constants::ProgramStartAddress}; // Program counter, starts at 0x200
        uint16_t m_Stack[constants::StackSize] {};      // Stack
        uint8_t  m_Memory[constants::MemorySize] {};    // Memory

        bool m_IsValid {false}; // Indicates if the Chip8 instance is valid
    };
} // namespace chip8cpp