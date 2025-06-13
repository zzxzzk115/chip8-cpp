#include "chip8cpp/chip8cpp.hpp"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace
{
    const uint8_t FontSet[chip8cpp::constants::FontSetSize] = {
        // Fontset data (0x0 to 0xF)
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
}

namespace chip8cpp
{
    Chip8::Chip8(const Config& config) : m_Config(config) {}

    void          Chip8::setConfig(const Config& config) { m_Config = config; }
    const Config& Chip8::getConfig() const { return m_Config; }

    bool Chip8::loadProgram(const std::string& fileName)
    {
        reset();

        // Load the program into memory starting at 0x200
        // Read file using C++ file I/O
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open())
        {
            return false; // Failed to open the file
        }
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        if (fileSize >
            (constants::MemorySize - constants::ProgramStartAddress)) // 4096 bytes total, 512 bytes reserved for system
        {
            return false; // Program too large to fit in memory
        }
        file.read(reinterpret_cast<char*>(&m_Memory[constants::ProgramStartAddress]), fileSize);
        if (!file)
        {
            return false; // Failed to read the file
        }
        file.close();

#ifdef DEBUG
        std::cout << "Loaded program: " << fileName << std::endl;
        std::cout << "Program size: " << fileSize << " bytes" << std::endl;
        std::cout << "Memory contents after loading program:" << std::endl;
        // Print used memory contents for debugging
        for (size_t i = constants::ProgramStartAddress; i < constants::ProgramStartAddress + fileSize; ++i)
        {
            std::cout << std::format("0x{:02X} ", m_Memory[i]);     // Print each byte in hex format
            if ((i - constants::ProgramStartAddress + 1) % 16 == 0) // New line every 16 bytes
            {
                std::cout << std::endl;
            }
        }
#endif

        m_IsValid = true;
        return true;
    }

    void Chip8::emulateOneCycle()
    {
        assert(m_IsValid);

        // Check if the Chip8 instance is valid before proceeding
        if (!m_IsValid)
        {
            std::cerr << "Chip8 instance is not valid. Please load a valid program first." << std::endl;
            return;
        }

        // Fetch the opcode from memory
        uint16_t opcode = fetchOpcode();

        // Decode and execute the opcode
        decodeAndExecuteOpcode(opcode);

        // Update timers
        updateTimers();

#ifdef DEBUG
        // Debug graphics buffer using CLI ASCII art
        if (m_Config.printAsciiGraphics)
        {
            std::cout << "\033[2J\033[1;1H";
            std::cout << "Graphics buffer state:" << std::endl;
            for (size_t y = 0; y < constants::Height; ++y)
            {
                for (size_t x = 0; x < constants::Width; ++x)
                {
                    size_t index = x + y * constants::Width;
                    std::cout << (m_GFX[index] ? '#' : '.'); // Print filled block for pixel on, space for pixel off
                }
                std::cout << std::endl; // New line after each row
            }
        }
#endif
    }

    bool Chip8::isKeyPressed(KeyCode keyCode) const { return m_Keys[static_cast<size_t>(keyCode)] != 0; }

    void Chip8::setKeyState(KeyCode keyCode, bool isPressed)
    {
        m_Keys[static_cast<size_t>(keyCode)] = isPressed ? 1 : 0;
    }

    bool Chip8::getDrawFlag() const { return m_DrawFlag; }

    const uint8_t* Chip8::getGFX() const { return m_GFX; }

    void Chip8::reset()
    {
        m_PC         = constants::ProgramStartAddress; // Program counter starts at 0x200
        m_SP         = 0;                              // Stack pointer
        m_I          = 0;                              // Index register
        m_DelayTimer = 0;                              // Delay timer
        m_SoundTimer = 0;                              // Sound timer
        m_DrawFlag   = false;                          // Reset draw flag

        std::fill(std::begin(m_V), std::end(m_V), 0);           // Clear registers
        std::fill(std::begin(m_Keys), std::end(m_Keys), 0);     // Clear key states
        std::fill(std::begin(m_GFX), std::end(m_GFX), 0);       // Clear graphics buffer
        std::fill(std::begin(m_Memory), std::end(m_Memory), 0); // Clear memory
        std::fill(std::begin(m_Stack), std::end(m_Stack), 0);   // Clear stack

        m_IsValid = false; // Reset validity

        // Load font set into memory
        for (size_t i = 0; i < constants::FontSetSize; ++i)
        {
            m_Memory[i] = FontSet[i];
        }
    }

    uint16_t Chip8::fetchOpcode() { return (m_Memory[m_PC] << 8) | m_Memory[m_PC + 1]; }

    void Chip8::decodeAndExecuteOpcode(uint16_t opcode)
    {
        // Decode the opcode and execute the corresponding instruction
        // https://en.wikipedia.org/wiki/CHIP-8
        switch (opcode & 0xF000)
        {
            case 0x0000: // 0x00E0: Clear the display
                std::fill(std::begin(m_GFX), std::end(m_GFX), 0);
                m_DrawFlag = true;
                m_PC += 2;
                break;

            case 0x1000: // 0x1NNN: Jump to address NNN
                m_PC = opcode & 0x0FFF;
                break;

            case 0x2000:                           // 0x2NNN: Call subroutine at NNN
                m_Stack[m_SP++] = m_PC;            // Push current PC onto stack
                m_PC            = opcode & 0x0FFF; // Set PC to NNN
                break;

            case 0x6000: // 0x6XNN: Set register VX to NN
                m_V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
                m_PC += 2;
                break;

            case 0x7000: // 0x7XNN: Add NN to register VX
                m_V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
                m_PC += 2;
                break;

            case 0xA000: // 0xANNN: Set index register I to NNN
                m_I = opcode & 0x0FFF;
                m_PC += 2;
                break;

            case 0xD000: // 0xDXYN: Draw sprite at (VX, VY) with height N
            {
                uint8_t x      = m_V[(opcode & 0x0F00) >> 8];
                uint8_t y      = m_V[(opcode & 0x00F0) >> 4];
                uint8_t height = opcode & 0x000F;
                m_V[0xF]       = 0; // Clear collision flag

                for (uint8_t row = 0; row < height; ++row)
                {
                    uint8_t pixel = m_Memory[m_I + row];
                    for (uint8_t col = 0; col < 8; ++col)
                    {
                        if ((pixel & (0x80 >> col)) != 0)
                        {
                            size_t gfxIndex = (x + col + (y + row) * constants::Width) % constants::GfxSize;
                            if (m_GFX[gfxIndex] == 1)
                                m_V[0xF] = 1;     // Collision detected
                            m_GFX[gfxIndex] ^= 1; // Toggle pixel
                        }
                    }
                }
                m_DrawFlag = true;
                m_PC += 2;
                break;
            }

                // TODO: Implement other opcodes as needed

            default:
                std::cout << std::format("Unknown opcode: 0x{:02X} at PC: 0x{:04X}", opcode, m_PC) << std::endl;
        }
    }

    void Chip8::updateTimers()
    {
        if (m_DelayTimer > 0)
            --m_DelayTimer;

        if (m_SoundTimer > 0)
        {
            if (m_SoundTimer == 1)
            {
#ifdef DEBUG
                std::cout << "BEEP! Sound timer reached zero." << std::endl;
#endif
                // TODO: Callback to play sound
            }
            --m_SoundTimer;
        }
    }

    void Chip8::loadFontSet()
    {
        // Load the Chip-8 font set into memory
        for (size_t i = 0; i < constants::FontSetSize; ++i)
        {
            m_Memory[i] = FontSet[i];
        }
    }
} // namespace chip8cpp