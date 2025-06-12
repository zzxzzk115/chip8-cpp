#pragma once

#include <string>

namespace chip8cpp
{
    struct Config
    {};

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

        void setConfig(const Config& config);

        bool loadProgram(const std::string& fileName);

        void emulateOneCycle();

        bool isKeyPressed(KeyCode keyCode) const;
        void setKeyState(KeyCode keyCode, bool isPressed);

        bool getDrawFlag() const;

    private:
        void reset();

        uint16_t fetchOpcode();
        void     decodeAndExecuteOpcode(uint16_t opcode);
        void     updateTimers();

        void loadFontSet();

    private:
        Config m_Config {}; // Configuration settings

        uint8_t  m_V[16] {};         // Registers
        uint8_t  m_DelayTimer {0};   // Delay timer
        uint8_t  m_SoundTimer {0};   // Sound timer
        uint8_t  m_SP {0};           // Stack pointer
        uint8_t  m_Keys[16] {};      // Key states, 0 for up, 1 for down, KeyCode is the index
        uint8_t  m_GFX[64 * 32] {};  // Graphics buffer (64x32 pixels)
        bool     m_DrawFlag {false}; // Flag to indicate if a redraw is needed
        uint16_t m_I {0};            // Index register
        uint16_t m_PC {0x200};       // Program counter, starts at 0x200
        uint16_t m_Stack[16] {};     // Stack
        uint8_t  m_Memory[4096] {};  // Memory

        bool m_IsValid {false}; // Indicates if the Chip8 instance is valid
    };
} // namespace chip8cpp