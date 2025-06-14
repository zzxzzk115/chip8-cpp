#include "chip8cpp/chip8cpp.hpp"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

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

    const char* getKeyCodeName(chip8cpp::KeyCode keyCode)
    {
        switch (keyCode)
        {
            case chip8cpp::KeyCode::eNum1:
                return "1";
            case chip8cpp::KeyCode::eNum2:
                return "2";
            case chip8cpp::KeyCode::eNum3:
                return "3";
            case chip8cpp::KeyCode::eC:
                return "C";
            case chip8cpp::KeyCode::eNum4:
                return "4";
            case chip8cpp::KeyCode::eNum5:
                return "5";
            case chip8cpp::KeyCode::eNum6:
                return "6";
            case chip8cpp::KeyCode::eD:
                return "D";
            case chip8cpp::KeyCode::eNum7:
                return "7";
            case chip8cpp::KeyCode::eNum8:
                return "8";
            case chip8cpp::KeyCode::eNum9:
                return "9";
            case chip8cpp::KeyCode::eE:
                return "E";
            case chip8cpp::KeyCode::eA:
                return "A";
            case chip8cpp::KeyCode::eNum0:
                return "0";
            case chip8cpp::KeyCode::eB:
                return "B";
            case chip8cpp::KeyCode::eF:
                return "F";
            default:
                assert(0);
                return "";
        }
    }
} // namespace

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

        // Debug input key states
        if (m_Config.printKeyStates)
        {
            for (size_t i = 0; i < constants::KeyCount; ++i)
            {
                const auto keyCode = static_cast<KeyCode>(i);
                if (isKeyPressed(keyCode))
                {
                    std::cout << std::format("Key {0} is pressed", getKeyCodeName(keyCode)) << std::endl;
                }
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
        // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#instructions
        // https://chip8.gulrak.net/
        // TODO: Implement opcodes for Super CHIP-8 and XO-CHIP-8
        switch (opcode & 0xF000)
        {
            case 0x0000:
                switch (opcode & 0x00FF)
                {
                    case 0x00E0: // 0x00E0: Clear the display
                        std::ranges::fill(m_GFX, 0);
                        m_DrawFlag = true;
                        m_PC += 2;
                        break;

                    case 0x00EE: // 0x00EE: Return from subroutine
                        if (m_SP == 0)
                        {
                            std::cerr << "Stack underflow on return from subroutine." << std::endl;
                            return;
                        }
                        m_PC = m_Stack[--m_SP] + 2; // Pop from stack and set PC
                        break;

                    default:
                        assert(0); // Unknown opcode
                }
                break;

            case 0x1000: // 0x1NNN: Jump to address NNN
                m_PC = opcode & 0x0FFF;
                break;

            case 0x2000:                           // 0x2NNN: Call subroutine at NNN
                m_Stack[m_SP++] = m_PC;            // Push current PC onto stack
                m_PC            = opcode & 0x0FFF; // Set PC to NNN
                break;

            case 0x3000: // 0x3XNN: Skip next instruction if VX == NN
                if (m_V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                {
                    m_PC += 4; // Skip next instruction
                }
                else
                {
                    m_PC += 2; // Move to next instruction
                }
                break;

            case 0x4000: // 0x4XNN: Skip next instruction if VX != NN
                if (m_V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                {
                    m_PC += 4; // Skip next instruction
                }
                else
                {
                    m_PC += 2; // Move to next instruction
                }
                break;

            case 0x5000: // 0x5XY0: Skip next instruction if VX == VY
                if (m_V[(opcode & 0x0F00) >> 8] == m_V[(opcode & 0x00F0) >> 4])
                {
                    m_PC += 4; // Skip next instruction
                }
                else
                {
                    m_PC += 2; // Move to next instruction
                }
                break;

            case 0x6000: // 0x6XNN: Set register VX to NN
                m_V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
                m_PC += 2;
                break;

            case 0x7000: // 0x7XNN: Add NN to register VX
                m_V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
                m_PC += 2;
                break;

            case 0x8000: // 0x8XY0: Set VX to VY
                switch (opcode & 0x000F)
                {
                    case 0x0000: // 0x8XY0: Set VX to VY
                        m_V[(opcode & 0x0F00) >> 8] = m_V[(opcode & 0x00F0) >> 4];
                        break;

                    case 0x0001: // 0x8XY1: Set VX to VX OR VY
                        m_V[(opcode & 0x0F00) >> 8] |= m_V[(opcode & 0x00F0) >> 4];
                        break;

                    case 0x0002: // 0x8XY2: Set VX to VX AND VY
                        m_V[(opcode & 0x0F00) >> 8] &= m_V[(opcode & 0x00F0) >> 4];
                        break;

                    case 0x0003: // 0x8XY3: Set VX to VX XOR VY
                        m_V[(opcode & 0x0F00) >> 8] ^= m_V[(opcode & 0x00F0) >> 4];
                        break;

                    case 0x0004: // 0x8XY4: Add VY to VX, set VF if carry
                    {
                        uint16_t sum                = m_V[(opcode & 0x0F00) >> 8] + m_V[(opcode & 0x00F0) >> 4];
                        m_V[0xF]                    = (sum > 255) ? 1 : 0; // Set carry flag
                        m_V[(opcode & 0x0F00) >> 8] = sum & 0xFF;          // Store result in VX
                        break;
                    }

                    case 0x0005: // 0x8XY5: Subtract VY from VX, set VF if no borrow
                    {
                        m_V[0xF] = (m_V[(opcode & 0x00F0) >> 4] <= m_V[(opcode & 0x0F00) >> 8]) ? 1 : 0;
                        m_V[(opcode & 0x0F00) >> 8] -= m_V[(opcode & 0x00F0) >> 4];
                        break;
                    }

                    case 0x0006: // 0x8XY6: Shift VX right by 1, set VF to LSB
                    {
                        m_V[0xF] = m_V[(opcode & 0x0F00) >> 8] & 0x01; // Store LSB in VF
                        m_V[(opcode & 0x0F00) >> 8] >>= 1;             // Shift right
                        break;
                    }

                    case 0x0007: // 0x8XY7: Set VX to VY - VX, set VF if no borrow
                    {
                        m_V[0xF] = (m_V[(opcode & 0x0F00) >> 8] <= m_V[(opcode & 0x00F0) >> 4]) ? 1 : 0;
                        m_V[(opcode & 0x0F00) >> 8] = m_V[(opcode & 0x00F0) >> 4] - m_V[(opcode & 0x0F00) >> 8];
                        break;
                    }

                    case 0x000E: // 0x8XYE: Shift VX left by 1, set VF to MSB
                    {
                        m_V[0xF] = (m_V[(opcode & 0x0F00) >> 8] & 0x80) >> 7; // Store MSB in VF
                        m_V[(opcode & 0x0F00) >> 8] <<= 1;                    // Shift left
                        break;
                    }

                    default:
                        assert(0); // Unknown opcode
                }
                m_PC += 2;
                break;

            case 0x9000: // 0x9XY0: Skip next instruction if VX != VY
                if (m_V[(opcode & 0x0F00) >> 8] != m_V[(opcode & 0x00F0) >> 4])
                {
                    m_PC += 4; // Skip next instruction
                }
                else
                {
                    m_PC += 2; // Move to next instruction
                }
                break;

            case 0xA000: // 0xANNN: Set index register I to NNN
                m_I = opcode & 0x0FFF;
                m_PC += 2;
                break;

            case 0xB000: // 0xBNNN: Jump to address NNN + V0
                m_PC = (opcode & 0x0FFF) + m_V[0];
                break;

            case 0xC000: // 0xCXNN: Set VX to random byte AND NN
            {
                std::random_device                 rd;
                std::mt19937                       gen(rd());
                std::uniform_int_distribution<int> dist(0, 255);
                uint8_t                            randomByte = static_cast<uint8_t>(dist(gen));
                m_V[(opcode & 0x0F00) >> 8]                   = randomByte & (opcode & 0x00FF);
                m_PC += 2;
                break;
            }

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

            case 0xE000: // 0xEXNN: Key operations
            {
                switch (opcode & 0x00FF)
                {
                    case 0x009E: // 0xEX9E: Skip next instruction if key VX is pressed
                        if (isKeyPressed(static_cast<KeyCode>(m_V[(opcode & 0x0F00) >> 8])))
                        {
                            m_PC += 4; // Skip next instruction
                        }
                        else
                        {
                            m_PC += 2; // Move to next instruction
                        }
                        break;

                    case 0x00A1: // 0xEXA1: Skip next instruction if key VX is not pressed
                        if (!isKeyPressed(static_cast<KeyCode>(m_V[(opcode & 0x0F00) >> 8])))
                        {
                            m_PC += 4; // Skip next instruction
                        }
                        else
                        {
                            m_PC += 2; // Move to next instruction
                        }
                        break;
                    default:
                        assert(0); // Unknown opcode
                }
                break;
            }

            case 0xF000: // 0xFXNN: Miscellaneous operations
            {
                switch (opcode & 0x00FF)
                {
                    case 0x0007: // 0xFX07: Set VX to delay timer value
                        m_V[(opcode & 0x0F00) >> 8] = m_DelayTimer;
                        m_PC += 2;
                        break;

                    case 0x000A: // 0xFX0A: Wait for key press, store in VX
                    {
                        bool keyPressed = false;
                        for (size_t i = 0; i < constants::KeyCount; ++i)
                        {
                            if (isKeyPressed(static_cast<KeyCode>(i)))
                            {
                                m_V[(opcode & 0x0F00) >> 8] = static_cast<uint8_t>(i);
                                keyPressed                  = true;
                                break;
                            }
                        }
                        if (!keyPressed)
                        {
                            return; // Wait for key press
                        }
                        m_PC += 2;
                        break;
                    }

                    case 0x0015: // 0xFX15: Set delay timer to VX
                        m_DelayTimer = m_V[(opcode & 0x0F00) >> 8];
                        m_PC += 2;
                        break;

                    case 0x0018: // 0xFX18: Set sound timer to VX
                        m_SoundTimer = m_V[(opcode & 0x0F00) >> 8];
                        m_PC += 2;
                        break;

                    case 0x001E: // 0xFX1E: Add VX to I
                        m_I += m_V[(opcode & 0x0F00) >> 8];
                        m_PC += 2;
                        break;

                    case 0x0029: // 0xFX29: Set I to the location of the sprite for digit VX
                    {
                        uint8_t digit = m_V[(opcode & 0x0F00) >> 8];
                        if (digit < constants::FontSetSize / constants::FontHeight)
                            m_I = digit * constants::FontHeight; // Each font character is stored in memory sequentially
                        else
                            std::cerr << "Invalid digit for sprite location." << std::endl;
                        m_PC += 2;
                        break;
                    }

                    case 0x0033: // 0xFX33: Store BCD representation of VX in memory at I
                    {
                        uint8_t value     = m_V[(opcode & 0x0F00) >> 8];
                        m_Memory[m_I]     = value / 100;       // Hundreds digit
                        m_Memory[m_I + 1] = (value / 10) % 10; // Tens digit
                        m_Memory[m_I + 2] = value % 10;        // Ones digit
                        m_PC += 2;
                        break;
                    }

                    case 0x0055: // 0xFX55: Store registers V0 to VX in memory starting at I
                    {
                        uint8_t x = (opcode & 0x0F00) >> 8;
                        for (uint8_t i = 0; i <= x; ++i)
                        {
                            m_Memory[m_I + i] = m_V[i];
                        }
                        m_I += x + 1; // Move I forward by the number of registers stored
                        m_PC += 2;
                        break;
                    }

                    case 0x0065: // 0xFX65: Read registers V0 to VX from memory starting at I
                    {
                        uint8_t x = (opcode & 0x0F00) >> 8;
                        for (uint8_t i = 0; i <= x; ++i)
                        {
                            m_V[i] = m_Memory[m_I + i];
                        }
                        m_I += x + 1; // Move I forward by the number of registers read
                        m_PC += 2;
                        break;
                    }

                    default:
                        assert(0); // Unknown opcode
                }
                break;
            }

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
                // Callback to play sound
                if (m_Config.soundCallback)
                {
                    m_Config.soundCallback();
                }
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