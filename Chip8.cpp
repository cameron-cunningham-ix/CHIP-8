#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <random>
#include <time.h>

#include "Chip8.h"

/// @brief Constructor - initializes FP tables for opcode functions
Chip8::Chip8() {
    // Set standard colors
    OnColor = 0xFFFFFFFF;
    OffColor = 0x000000FF;
    // Setup function pointer tables for opcode funcs
    FnTable[0x0] = &Chip8::Table0;
    FnTable[0x1] = &Chip8::op_1nnn;
    FnTable[0x2] = &Chip8::op_2nnn;
    FnTable[0x3] = &Chip8::op_3xnn;
    FnTable[0x4] = &Chip8::op_4xnn;
    FnTable[0x5] = &Chip8::op_5xy0;
    FnTable[0x6] = &Chip8::op_6xnn;
    FnTable[0x7] = &Chip8::op_7xnn;
    FnTable[0x8] = &Chip8::Table8;
    FnTable[0x9] = &Chip8::op_9xy0;
    FnTable[0xA] = &Chip8::op_annn;
    FnTable[0xB] = &Chip8::op_bnnn;
    FnTable[0xC] = &Chip8::op_cxnn;
    FnTable[0xD] = &Chip8::op_dxyn;
    FnTable[0xE] = &Chip8::TableE;
    FnTable[0xF] = &Chip8::TableF;

    for (int i = 0; i < 16; i++)
    {
        FnTable0[i] = &Chip8::op_null;
        FnTable8[i] = &Chip8::op_null;
        FnTableE[i] = &Chip8::op_null;
    }

    FnTable0[0x0] = &Chip8::op_00e0;
    FnTable0[0xE] = &Chip8::op_00ee;

    FnTable8[0x0] = &Chip8::op_8xy0;
    FnTable8[0x1] = &Chip8::op_8xy1;
    FnTable8[0x2] = &Chip8::op_8xy2;
    FnTable8[0x3] = &Chip8::op_8xy3;
    FnTable8[0x4] = &Chip8::op_8xy4;
    FnTable8[0x5] = &Chip8::op_8xy5;
    FnTable8[0x6] = &Chip8::op_8xy6;
    FnTable8[0x7] = &Chip8::op_8xy7;
    FnTable8[0xE] = &Chip8::op_8xyE;

    FnTableE[0x1] = &Chip8::op_exa1;
    FnTableE[0xE] = &Chip8::op_ex9e;

    for (size_t i = 0; i <= 0x65; i++)
    {
        FnTableF[i] = &Chip8::op_null;
    }

    FnTableF[0x07] = &Chip8::op_fx07;
    FnTableF[0x0A] = &Chip8::op_fx0a;
    FnTableF[0x15] = &Chip8::op_fx15;
    FnTableF[0x18] = &Chip8::op_fx18;
    FnTableF[0x1E] = &Chip8::op_fx1e;
    FnTableF[0x29] = &Chip8::op_fx29;
    FnTableF[0x33] = &Chip8::op_fx33;
    FnTableF[0x55] = &Chip8::op_fx55;
    FnTableF[0x65] = &Chip8::op_fx65;
}

Chip8::~Chip8() {}

/// @brief Initialize memory, registers, and display to 0.
/// Start PC at PROGRAM_START, 0x0200
void Chip8::initialize()
{
    // Zero out RAM
    memset(RAM, 0, 4096);
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
    memcpy(RAM + FONT_START, font, 80);
    // Zero out registers, keys, stack, display
    memset(V, 0, 16);
    memset(Keys, 0, 16);
    memset(PrevKeys, 0, 16);
    memset(Stack, 0, 16*sizeof(uint_16));
    // NOTE: Can't use memset when trying to fill with values larger
    // than 1 byte. Lesson learned.
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        Display[i] = OffColor;
    }

    SP = 0;
    I = 0;
    Opcode = 0;
    DelayTimer = 0;
    SoundTimer = 0;
    // Start program counter at 0x0200
    PC = PROGRAM_START;
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
    fread(RAM + PROGRAM_START, sizeof(uint_8), 4096 - PROGRAM_START, rom);
    // Close file pointer
    fclose(rom);
    return true;
}

void Chip8::storePrevValues()
{
    memcpy(PrevStack, Stack, 16*sizeof(uint_16));
    memcpy(PrevV, V, 16*sizeof(uint_8));
    PrevSP = SP;
    PrevPC = PC;
    PrevI = I;
    PrevOpcode = Opcode;
}

/// @brief Emulates one CHIP-8 cycle
void Chip8::cycle()
{
    storePrevValues();
    // Read the next 16-bit instruction
    Opcode = (RAM[PC] << 8) | RAM[PC + 1];
    // Increment PC by 2 to be ready for next opcode
    PC += 2;

    // Get Opcode helper values
    Nib1 = Opcode & 0xF000;         // First nibble (half byte)
    Nib2 = Opcode & 0x000F;         // Last nibble
    Vx = (Opcode & 0x0F00) >> 8;    // Register x, not value in Vx
    Vy = (Opcode & 0x00F0) >> 4;    // Register y, not value in Vy
    // Reset DrawFlag
    DrawFlag = 0;

    // Decode and Execute
    // Indexes the FnTable holding the function pointers for opcodes
    ((*this).*(FnTable[Nib1 >> 12]))();

    // Decrement delay and sound timers
    if (DelayTimer > 0)
        DelayTimer--;
    if (SoundTimer > 0)
        SoundTimer--;
}

/// @brief Set the 'on' color and write it to display
/// @param color - RRGGBBAA format
void Chip8::setOnColor(unsigned int color)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        if (Display[i] == OnColor) Display[i] = color;
    }
    OnColor = color;
    DrawFlag = 1;
}

/// @brief Set the 'off' color and write it to display
/// @param color - RRGGBBAA format
void Chip8::setOffColor(unsigned int color)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        if (Display[i] == OffColor) Display[i] = color;
    }
    OffColor = color;
    DrawFlag = 1;
}

// Table functions
void Chip8::Table0()
{
    ((*this).*(FnTable0[Nib2]))();
}
void Chip8::Table8()
{
    ((*this).*(FnTable8[Nib2]))();
}
void Chip8::TableE()
{
    ((*this).*(FnTableE[Nib2]))();
}
void Chip8::TableF()
{
    // Table F relies on last byte for indexing rather than nibble
    ((*this).*(FnTableF[Opcode & 0x00FF]))();
}

/// @brief Clear display
void Chip8::op_00e0()
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        Display[i] = OffColor;
    }
    DrawFlag = 1;
}
/// @brief Return call
void Chip8::op_00ee()
{
    SP--;
    PC = Stack[SP];
}
/// @brief Jump - Jump to address NNN
void Chip8::op_1nnn()
{
    PC = Opcode & 0x0FFF;
}
/// @brief Call addr
void Chip8::op_2nnn()
{
    // Push current PC onto stack and inc SP
    Stack[SP] = PC;
    SP++;
    // Jump to NNN
    PC = Opcode & 0x0FFF;
}
/// @brief Skip instruction if Vx val == NN
void Chip8::op_3xnn()
{
    if (V[Vx] == (Opcode & 0x00FF))
    {
        PC += 2;
    }
}
/// @brief Skip instruction if Vx val != NN
void Chip8::op_4xnn()
{
    if (V[Vx] != (Opcode & 0x00FF))
    {
        PC += 2;  
    } 
}
/// @brief Skip instruction if Vx == Vy
void Chip8::op_5xy0()
{
    if (V[Vx] == V[Vy])
    {
        PC += 2;
    }
}
/// @brief 6xNN - Set register Vx to NN
void Chip8::op_6xnn()
{
    V[Vx] = Opcode & 0x00FF;
}
/// @brief 7xNN - Add NN to register Vx
void Chip8::op_7xnn()
{
    V[Vx] += (Opcode & 0x00FF);
}
// Logical / arithmetic ops
/// @brief Set Vx to Vy
void Chip8::op_8xy0()
{
    V[Vx] = V[Vy];
}
/// @brief Binary OR
void Chip8::op_8xy1()
{
    V[Vx] = V[Vx] | V[Vy];
}
/// @brief Binary AND
void Chip8::op_8xy2()
{
    V[Vx] = V[Vx] & V[Vy];
}
/// @brief Logical XOR
void Chip8::op_8xy3()
{
    V[Vx] = V[Vx] ^ V[Vy];
}
/// @brief Add with overflow detect
void Chip8::op_8xy4()
{
    uint_8 x = V[Vx];
    V[Vx] = V[Vx] + V[Vy];
    // Overflow, set VF to 1
    if (x > V[Vx]) V[15] = 0x01;
    else V[15] = 0;
}
/// @brief Subtract Vx - Vy
void Chip8::op_8xy5()
{
    uint_8 x = V[Vx];
    V[Vx] = V[Vx] - V[Vy];
    if (V[Vx] > x)  V[15] = 0x00;   // "Underflowed"
    else V[15] = 0x01;
}
/// @brief Shift right
void Chip8::op_8xy6()
{
    if (!ConfigShift)
    {
        V[Vx] = V[Vy];
    }
    uint_8 x = V[Vx];
    V[Vx] = x >> 1;
    V[15] = (x & 0x01);
}
/// @brief Subtract Vy - Vx
void Chip8::op_8xy7()
{
    uint_8 x = V[Vy];
    V[Vx] = V[Vy] - V[Vx];
    if (V[Vx] > x)  V[15] = 0x00;   // "Underflowed"
    else V[15] = 0x01;
}
/// @brief Shift left
void Chip8::op_8xyE()
{
    if (!ConfigShift)
    {
        V[Vx] = V[Vy];
    }
    uint_8 x = V[Vx];
    V[Vx] = x << 1;
    V[15] = (x & 0x80) >> 7;
}
/// @brief Skip instruction if Vx != Vy
void Chip8::op_9xy0()
{
    if (V[Vx] != V[Vy]) PC += 2;
}
/// @brief Set index register I to NNN
void Chip8::op_annn()
{
    I = Opcode & 0x0FFF;
}
/// @brief Jump with offset
void Chip8::op_bnnn()
{
    if (!ConfigJumpWOffset)
    {
        PC = Opcode & 0x0FFF + V[0];
    } else {
        PC = Opcode & 0x0FFF + V[Vx];
    }
}
/// @brief Random
void Chip8::op_cxnn()
{
    uint_8 x = (uint_8)rand();
    x &= (Opcode & 0x00FF);
    V[Vx] = x;
}
/// @brief Draw N-byte sprite
void Chip8::op_dxyn()
{
    uint_8 height = Opcode & 0x000F;     // Height of the sprite to be drawn

    // Get X-Y coords
    uint_16 x0 = V[Vx] & 63;
    uint_16 y0 = V[Vy] & 31;

    // Set VF to 0
    V[15] = 0;
    // For N rows:
    for (uint_16 row = 0; row < height; row++)
    {
        // Get the next byte in sprite
        uint_8 spriteByte = RAM[I + row];

        // For each pixel/bit
        for (uint_16 col = 0; col < 8; col++)
        {
            // Current pixel/bit in sprite
            uint_16 spritePixel = spriteByte & (0x80 >> col);

            // X-Y coords of current pixel on display
            uint_16 px = (x0 + col) & 63;
            uint_16 py = (y0 + row) & 31;

            // If this pixel in sprite is on and the screen pixel is on
            if (spritePixel && Display[px + py*64] == OnColor)
            {
                // XOR/turnoff screen pixel and set collision flag with VF
                Display[px + py*64] = OffColor;
                V[15] = 0x01;
            }
            else if (spritePixel)   // If pixel in sprite is on and screen pixel is off
            {
                Display[px + py*64] = OnColor;
            }
            // If you reach right edge of screen break; CHIP-8 should not wrap sprites
            if (x0 + col >= 63) break;
        }
        // If you reach bottom of screen
        if (y0 + row >= 31) break;
    }
    DrawFlag = 1;
}
/// @brief Skip if key corresponding to value in Vx is pressed
void Chip8::op_ex9e()
{
    if (Keys[V[Vx]]) PC += 2;
}
/// @brief Skip if key corresponding to value in Vx is not pressed
void Chip8::op_exa1()
{
    if (!Keys[V[Vx]]) PC += 2;
}
/// @brief Set Vx to value of delay timer
void Chip8::op_fx07()
{
    V[Vx] = DelayTimer;
}
/// @brief Wait until key is released
void Chip8::op_fx0a()
{
    PC -= 2;
    for (uint_8 i = 0; i < 16; i++)
    {
        if (Keys[i] == 0 && PrevKeys[i] == 1) 
        {
            V[Vx] = i;
            PC += 2;
            break;
        }
    }
}
/// @brief Set delay timer to value in Vx
void Chip8::op_fx15()
{
    DelayTimer = V[Vx];
}
/// @brief Set sound timer to value in Vx
void Chip8::op_fx18()
{
    SoundTimer = V[Vx];
}
/// @brief Add to index register
void Chip8::op_fx1e()
{
    I += V[Vx];
    // NOTE: OG COSMAC VIP interpeter did not set VF if I "overflows", but Amiga's did.
    if (I > 0x0FFF) V[15] = 0x01;
}
/// @brief Font character
void Chip8::op_fx29()
{
    // I = RAM[(V[Vx] & 0x000F)*5 + FONT_START];
    I = (V[Vx] & 0x000F)*5 + FONT_START;
}
/// @brief Binary-coded decimal conversion
void Chip8::op_fx33()
{
    uint_8 x = V[Vx];
    RAM[I + 2] = x % 10;
    uint_8 y = x / 10;
    RAM[I + 1] = y % 10;
    y /= 10;
    RAM[I] = y;
}
/// @brief Store memory
void Chip8::op_fx55()
{
    for (uint_8 i = 0; i <= Vx; i++)
    {
        RAM[I + i] = V[i];
    }
}
/// @brief Load memory
void Chip8::op_fx65()
{
    for (uint_8 i = 0; i <= Vx; i++)
    {
        V[i] = RAM[I + i];
    }
}
/// @brief Default function if opcode doesn't exist
void Chip8::op_null()
{
}
