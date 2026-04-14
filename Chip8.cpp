#include <iostream>
#include <random>
#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "Chip8.h"

/// @brief Constructor - initializes FP tables for opcode functions
Chip8::Chip8() {
    // Initial Configuration
    configShift = true;
    configJumpWOffset = true;
    // Set standard colors
    onColor = 0xFFFFFFFF;
    offColor = 0x000000FF;
    // Setup function pointer tables for opcode funcs
    fnTable[0x0] = &Chip8::table0;
    fnTable[0x1] = &Chip8::op_1nnn;
    fnTable[0x2] = &Chip8::op_2nnn;
    fnTable[0x3] = &Chip8::op_3xnn;
    fnTable[0x4] = &Chip8::op_4xnn;
    fnTable[0x5] = &Chip8::op_5xy0;
    fnTable[0x6] = &Chip8::op_6xnn;
    fnTable[0x7] = &Chip8::op_7xnn;
    fnTable[0x8] = &Chip8::table8;
    fnTable[0x9] = &Chip8::op_9xy0;
    fnTable[0xA] = &Chip8::op_annn;
    fnTable[0xB] = &Chip8::op_bnnn;
    fnTable[0xC] = &Chip8::op_cxnn;
    fnTable[0xD] = &Chip8::op_dxyn;
    fnTable[0xE] = &Chip8::tableE;
    fnTable[0xF] = &Chip8::tableF;

    for (int i = 0; i < 16; i++)
    {
        fnTable0[i] = &Chip8::op_null;
        fnTable8[i] = &Chip8::op_null;
        fnTableE[i] = &Chip8::op_null;
    }

    fnTable0[0x0] = &Chip8::op_00e0;
    fnTable0[0xE] = &Chip8::op_00ee;

    fnTable8[0x0] = &Chip8::op_8xy0;
    fnTable8[0x1] = &Chip8::op_8xy1;
    fnTable8[0x2] = &Chip8::op_8xy2;
    fnTable8[0x3] = &Chip8::op_8xy3;
    fnTable8[0x4] = &Chip8::op_8xy4;
    fnTable8[0x5] = &Chip8::op_8xy5;
    fnTable8[0x6] = &Chip8::op_8xy6;
    fnTable8[0x7] = &Chip8::op_8xy7;
    fnTable8[0xE] = &Chip8::op_8xyE;

    fnTableE[0x1] = &Chip8::op_exa1;
    fnTableE[0xE] = &Chip8::op_ex9e;

    for (size_t i = 0; i <= 0x65; i++)
    {
        fnTableF[i] = &Chip8::op_null;
    }

    fnTableF[0x07] = &Chip8::op_fx07;
    fnTableF[0x0A] = &Chip8::op_fx0a;
    fnTableF[0x15] = &Chip8::op_fx15;
    fnTableF[0x18] = &Chip8::op_fx18;
    fnTableF[0x1E] = &Chip8::op_fx1e;
    fnTableF[0x29] = &Chip8::op_fx29;
    fnTableF[0x33] = &Chip8::op_fx33;
    fnTableF[0x55] = &Chip8::op_fx55;
    fnTableF[0x65] = &Chip8::op_fx65;

    initialize();
}

Chip8::~Chip8() {}

/// @brief Initialize memory, registers, and display to 0.
/// Start PC at PROGRAM_START, 0x0200
void Chip8::initialize()
{
    // Zero out RAM
    memset(ram, 0, 4096);
    // Write font into RAM
    uint_8 font[80] = {
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
    memcpy(ram + FONT_START, font, 80);
    // Zero out registers, keys, stack, display
    memset(v, 0, 16);
    memset(keys, 0, 16);
    memset(prevKeys, 0, 16);
    memset(stack, 0, 16*sizeof(uint_16));
    // NOTE: Can't use memset when trying to fill with values larger
    // than 1 byte. Lesson learned.
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        display[i] = offColor;
    }

    sp = 0;
    idx = 0;
    opcode = 0;
    delayTimer = 0;
    soundTimer = 0;
    // Start program counter at 0x0200
    pc = PROGRAM_START;
    // Start RNG
    srand(time(NULL));
};

/// @brief Initialize CHIP-8 and loads ROM into CHIP-8 RAM
bool Chip8::loadROM(char* filePath)
{
    // Initialize CHIP-8 memory
    initialize();

    FILE *rom = fopen(filePath, "rb");

    if (rom == NULL)
    {
        printf("File not opened.");
        return false;
    }

    // Write program to RAM
    fread(ram + PROGRAM_START, sizeof(uint_8), 4096 - PROGRAM_START, rom);
    // Close file pointer
    fclose(rom);
    return true;
}

void Chip8::storePrevValues()
{
    memcpy(prevStack, stack, 16*sizeof(uint_16));
    memcpy(prevV, v, 16*sizeof(uint_8));
    prevSP = sp;
    prevPC = pc;
    prevIdx = idx;
    prevOpcode = opcode;
}

/// @brief Emulates one CHIP-8 cycle
void Chip8::cycle()
{
    storePrevValues();
    // Read the next 16-bit instruction
    opcode = (ram[pc] << 8) | ram[pc + 1];
    // Increment PC by 2 to be ready for next opcode
    pc += 2;

    // Get opcode helper values
    nib1 = opcode & 0xF000;         // First nibble (half byte)
    nib2 = opcode & 0x000F;         // Last nibble
    vx = (opcode & 0x0F00) >> 8;    // Register x, not value in vx
    vy = (opcode & 0x00F0) >> 4;    // Register y, not value in vy
    // Reset drawFlag
    // drawFlag = 0;

    // Decode and Execute
    // Indexes the fnTable holding the function pointers for opcodes
    ((*this).*(fnTable[nib1 >> 12]))();
}

/// @brief Set the 'on' color and write it to display
/// @param color - RRGGBBAA format
void Chip8::setOnColor(unsigned int color)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        if (display[i] == onColor) display[i] = color;
    }
    onColor = color;
    drawFlag = 1;
}

/// @brief Set the 'off' color and write it to display
/// @param color - RRGGBBAA format
void Chip8::setOffColor(unsigned int color)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        if (display[i] == offColor) display[i] = color;
    }
    offColor = color;
    drawFlag = 1;
}

// Table functions
void Chip8::table0()
{
    if (nib2 > 0xE)
    {
        std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << "\n";
        return;
    }
    ((*this).*(fnTable0[nib2]))();
}
void Chip8::table8()
{
    if (nib2 > 0xE)
    {
        std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << "\n";
        return;
    }
    ((*this).*(fnTable8[nib2]))();
}
void Chip8::tableE()
{
    if (nib2 > 0xE)
    {
        std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << "\n";
        return;
    }
    ((*this).*(fnTableE[nib2]))();
}
void Chip8::tableF()
{
    if ((opcode & 0x00FF) > 0x65)
    {
        std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << "\n";
        return;
    }
    // Table F relies on last byte for indexing rather than nibble
    ((*this).*(fnTableF[opcode & 0x00FF]))();
}

/// @brief Clear display
void Chip8::op_00e0()
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        display[i] = offColor;
    }
    drawFlag = 1;
}
/// @brief Return call
void Chip8::op_00ee()
{
    sp--;
    pc = stack[sp];
}
/// @brief Jump - Jump to address NNN
void Chip8::op_1nnn()
{
    pc = opcode & 0x0FFF;
}
/// @brief Call addr
void Chip8::op_2nnn()
{
    // Push current PC onto stack and inc sp
    stack[sp] = pc;
    sp++;
    // Jump to NNN
    pc = opcode & 0x0FFF;
}
/// @brief Skip instruction if vx val == NN
void Chip8::op_3xnn()
{
    if (v[vx] == (opcode & 0x00FF))
    {
        pc += 2;
    }
}
/// @brief Skip instruction if vx val != NN
void Chip8::op_4xnn()
{
    if (v[vx] != (opcode & 0x00FF))
    {
        pc += 2;  
    } 
}
/// @brief Skip instruction if vx == vy
void Chip8::op_5xy0()
{
    if (v[vx] == v[vy])
    {
        pc += 2;
    }
}
/// @brief 6xNN - Set register vx to NN
void Chip8::op_6xnn()
{
    v[vx] = opcode & 0x00FF;
}
/// @brief 7xNN - Add NN to register vx
void Chip8::op_7xnn()
{
    v[vx] += (opcode & 0x00FF);
}
// Logical / arithmetic ops
/// @brief Set vx to vy
void Chip8::op_8xy0()
{
    v[vx] = v[vy];
}
/// @brief Binary OR
void Chip8::op_8xy1()
{
    v[vx] = v[vx] | v[vy];
}
/// @brief Binary AND
void Chip8::op_8xy2()
{
    v[vx] = v[vx] & v[vy];
}
/// @brief Logical XOR
void Chip8::op_8xy3()
{
    v[vx] = v[vx] ^ v[vy];
}
/// @brief Add with overflow detect
void Chip8::op_8xy4()
{
    uint_8 x = v[vx];
    v[vx] = v[vx] + v[vy];
    // Overflow, set VF to 1
    if (x > v[vx]) v[15] = 0x01;
    else v[15] = 0;
}
/// @brief Subtract vx - vy
void Chip8::op_8xy5()
{
    uint_8 x = v[vx];
    v[vx] = v[vx] - v[vy];
    if (v[vx] > x)  v[15] = 0x00;   // "Underflowed"
    else v[15] = 0x01;
}
/// @brief Shift right
void Chip8::op_8xy6()
{
    if (!configShift)
    {
        // Original CHIP-8 behavior
        v[vx] = v[vy];
    }
    uint_8 x = v[vx];
    v[vx] = x >> 1;
    v[15] = (x & 0x01);
}
/// @brief Subtract vy - vx
void Chip8::op_8xy7()
{
    uint_8 x = v[vy];
    v[vx] = v[vy] - v[vx];
    if (v[vx] > x)  v[15] = 0x00;   // "Underflowed"
    else v[15] = 0x01;
}
/// @brief Shift left
void Chip8::op_8xyE()
{
    if (!configShift)
    {
        // Original CHIP-8 behavior
        v[vx] = v[vy];
    }
    uint_8 x = v[vx];
    v[vx] = x << 1;
    v[15] = (x & 0x80) >> 7;
}
/// @brief Skip instruction if vx != vy
void Chip8::op_9xy0()
{
    if (v[vx] != v[vy]) pc += 2;
}
/// @brief Set index register idx to NNN
void Chip8::op_annn()
{
    idx = opcode & 0x0FFF;
}
/// @brief Jump with offset
void Chip8::op_bnnn()
{
    if (!configJumpWOffset)
    {
        // Original CHIP-8 behavior
        pc = opcode & 0x0FFF + v[0];
    } else {
        pc = opcode & 0x0FFF + v[vx];
    }
}
/// @brief Random
void Chip8::op_cxnn()
{
    uint_8 x = (uint_8)rand();
    x &= (opcode & 0x00FF);
    v[vx] = x;
}
/// @brief Draw N-byte sprite
void Chip8::op_dxyn()
{
    uint_8 height = opcode & 0x000F;     // Height of the sprite to be drawn

    // Get X-Y coords
    uint_16 x0 = v[vx] & 63;
    uint_16 y0 = v[vy] & 31;

    // Set VF to 0
    v[15] = 0;
    // For N rows:
    for (uint_16 row = 0; row < height; row++)
    {
        // Get the next byte in sprite
        uint_8 spriteByte = ram[idx + row];

        // For each pixel/bit
        for (uint_16 col = 0; col < 8; col++)
        {
            // Current pixel/bit in sprite
            uint_16 spritePixel = spriteByte & (0x80 >> col);

            // X-Y coords of current pixel on display
            uint_16 px = (x0 + col) & 63;
            uint_16 py = (y0 + row) & 31;

            // If this pixel in sprite is on and the screen pixel is on
            if (spritePixel && display[px + py*64] == onColor)
            {
                // XOR/turnoff screen pixel and set collision flag with VF
                display[px + py*64] = offColor;
                v[15] = 0x01;
            }
            else if (spritePixel)   // If pixel in sprite is on and screen pixel is off
            {
                display[px + py*64] = onColor;
            }
            // If you reach right edge of screen break; CHIP-8 should not wrap sprites
            if (x0 + col >= 63) break;
        }
        // If you reach bottom of screen
        if (y0 + row >= 31) break;
    }
    drawFlag = 1;
}
/// @brief Skip if key corresponding to value in vx is pressed
void Chip8::op_ex9e()
{
    if (keys[v[vx]]) pc += 2;
}
/// @brief Skip if key corresponding to value in vx is not pressed
void Chip8::op_exa1()
{
    if (!keys[v[vx]]) pc += 2;
}
/// @brief Set vx to value of delay timer
void Chip8::op_fx07()
{
    v[vx] = delayTimer;
}
/// @brief Wait until key is released
void Chip8::op_fx0a()
{
    pc -= 2;
    for (uint_8 i = 0; i < 16; i++)
    {
        if (keys[i] == 0 && prevKeys[i] == 1) 
        {
            v[vx] = i;
            pc += 2;
            break;
        }
    }
}
/// @brief Set delay timer to value in vx
void Chip8::op_fx15()
{
    delayTimer = v[vx];
}
/// @brief Set sound timer to value in vx
void Chip8::op_fx18()
{
    soundTimer = v[vx];
}
/// @brief Add to index register
void Chip8::op_fx1e()
{
    idx += v[vx];
    // NOTE: OG COSMAC VIP interpeter did not set VF if idx "overflows", but Amiga's did.
    if (idx > 0x0FFF) v[15] = 0x01;
}
/// @brief Font character
void Chip8::op_fx29()
{
    // idx = RAM[(v[vx] & 0x000F)*5 + FONT_START];
    idx = (v[vx] & 0x000F)*5 + FONT_START;
}
/// @brief Binary-coded decimal conversion
void Chip8::op_fx33()
{
    uint_8 x = v[vx];
    ram[idx + 2] = x % 10;
    uint_8 y = x / 10;
    ram[idx + 1] = y % 10;
    y /= 10;
    ram[idx] = y;
}
/// @brief Store memory
void Chip8::op_fx55()
{
    for (uint_8 i = 0; i <= vx; i++)
    {
        ram[idx + i] = v[i];
    }
}
/// @brief Load memory
void Chip8::op_fx65()
{
    for (uint_8 i = 0; i <= vx; i++)
    {
        v[i] = ram[idx + i];
    }
}
/// @brief Default function if opcode doesn't exist
void Chip8::op_null()
{
    std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << "\n";
    return;
}
