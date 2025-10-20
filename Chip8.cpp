#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <random>
#include <time.h>

#include "Chip8.h"

// Constructor / Destructor
Chip8::Chip8() {
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

void Chip8::initialize()
{
    // Zero out RAM
    memset(RAM, 0x00, 4096);
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
    memset(V, 0x00, 16);
    memset(Keys, 0x00, 16);
    memset(Stack, 0x0000, 16*sizeof(uint_16));
    memset(Display, 0x00, 64*32);

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

// Initialize CHIP-8 and loads ROM into memory
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

void Chip8::cycle()
{
    // Read the next 16-bit instruction
    Opcode = (RAM[PC] << 8) | RAM[PC + 1];
    // Increment PC by 2 to be ready for next opcode
    PC += 2;

    // Get Opcode helper values
    Nib1 = Opcode & 0xF000;         // First nibble (half byte)
    Nib2 = Opcode & 0x000F;         // Last nibble
    Vx = (Opcode & 0x0F00) >> 8;    // Register x, not value in Vx
    Vy = (Opcode & 0x00F0) >> 4;    // Register y, not value in Vy

    // Decode and Execute
    // Indexes the FnTable holding the function pointers for opcodes
    ((*this).*(FnTable[Nib1 >> 12]))();

    // Decrement delay and sound timers
    if (DelayTimer > 0)
        DelayTimer--;
    if (SoundTimer > 0)
        SoundTimer--;
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
    ((*this).*(FnTableF[Nib2]))();
}

// Clear display
void Chip8::op_00e0()
{
    memset(Display, 0x00, 64*32);
}
// Return call
void Chip8::op_00ee()
{
    SP--;
    PC = Stack[SP];
}
// Jump - Jump to address NNN
void Chip8::op_1nnn()
{
    PC = Opcode & 0x0FFF;
}
// Call addr
void Chip8::op_2nnn()
{
    // Push current PC onto stack and inc SP
    Stack[SP] = PC;
    SP++;
    // Jump to NNN
    PC = Opcode & 0x0FFF;
}
// Skip instruction if Vx val == NN
void Chip8::op_3xnn()
{
    if (V[Vx] == (Opcode & 0x00FF))
    {
        PC += 2;
    }
}
// Skip instruction if Vx val != NN
void Chip8::op_4xnn()
{
    if (V[Vx] != (Opcode & 0x00FF))
    {
        PC += 2;  
    } 
}
// Skip instruction if Vx == Vy
void Chip8::op_5xy0()
{
    if (V[Vx] == V[Vy])
    {
        PC += 2;
    }
}
// 6xNN - Set register Vx to NN
void Chip8::op_6xnn()
{
    V[Vx] = Opcode & 0x00FF;
}
// 7xNN - Add NN to register Vx
void Chip8::op_7xnn()
{
    V[Vx] += (Opcode & 0x00FF);
}
// Logical / arithmetic ops
// Set Vx to Vy
void Chip8::op_8xy0()
{
    V[Vx] = V[Vy];
}
// Binary OR
void Chip8::op_8xy1()
{
    V[Vx] = V[Vx] | V[Vy];
}
// Binary AND
void Chip8::op_8xy2()
{
    V[Vx] = V[Vx] & V[Vy];
}
// Logical XOR
void Chip8::op_8xy3()
{
    V[Vx] = V[Vx] ^ V[Vy];
}
// Add with overflow detect
void Chip8::op_8xy4()
{
    uint_8 x = V[Vx];
    V[Vx] = V[Vx] + V[Vy];
    // Overflow, set VF to 1
    if (x > V[Vx]) V[15] = 0x01;
    else V[15] = 0;
}
// Subtract Vx - Vy
void Chip8::op_8xy5()
{
    uint_8 x = V[Vx];
    V[Vx] = V[Vx] - V[Vy];
    if (V[Vx] > x)  V[15] = 0x00;   // "Underflowed"
    else V[15] = 0x01;
}
// Shift right
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
// Subtract Vy - Vx
void Chip8::op_8xy7()
{
    uint_8 x = V[Vy];
    V[Vx] = V[Vy] - V[Vx];
    if (V[Vx] > x)  V[15] = 0x00;   // "Underflowed"
    else V[15] = 0x01;
}
// Shift left
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
// Skip instruction if Vx != Vy
void Chip8::op_9xy0()
{
    if (V[Vx] != V[Vy]) PC += 2;
}
// Set index register I to NNN
void Chip8::op_annn()
{
    I = Opcode & 0x0FFF;
}
// Jump with offset
void Chip8::op_bnnn()
{
    if (!ConfigJumpWOffset)
    {
        PC = Opcode & 0x0FFF + V[0];
    } else {
        PC = Opcode & 0x0FFF + V[Vx];
    }
}
// Random
void Chip8::op_cxnn()
{
    uint_8 x = (uint_8)rand();
    x &= (Opcode & 0x0FF0);
    V[Vx] = x;
}
// Draw N-byte sprite
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
            if (spritePixel && Display[px + py*64] == 0x01)
            {
                // XOR/turnoff screen pixel and set collision flag with VF
                Display[px + py*64] = 0x00;
                V[15] = 0x01;
            }
            else if (spritePixel)   // If pixel in sprite is on and screen pixel is off
            {
                Display[px + py*64] = 0x01;
            }
            // If you reach right edge of screen break; CHIP-8 should not wrap sprites
            if (x0 + col >= 63) break;
        }
        // If you reach bottom of screen
        if (y0 + row >= 31) break;
    }
}
// Skip if key corresponding to value in Vx is pressed
void Chip8::op_ex9e()
{
    if (Keys[V[Vx]]) PC += 2;
}
// Skip if key corresponding to value in Vx is not pressed
void Chip8::op_exa1()
{
    if (!Keys[V[Vx]]) PC += 2;
}
// Set Vx to value of delay timer
void Chip8::op_fx07()
{
    V[Vx] = DelayTimer;
}
// Get key
void Chip8::op_fx0a()
{
    PC -= 2;
    for (uint_8 i = 0; i < 16; i++)
    {
        if (Keys[i] = 1) 
        {
            V[Vx] = i;
            PC += 2;
            break;
        }
    }
}
// Set delay timer to value in Vx
void Chip8::op_fx15()
{
    DelayTimer = V[Vx];
}
// Set sound timer to value in Vx
void Chip8::op_fx18()
{
    SoundTimer = V[Vx];
}
// Add to index register
void Chip8::op_fx1e()
{
    I += V[Vx];
    // NOTE: OG COSMAC VIP interpeter did not set VF if I "overflows", but Amiga's did.
    if (I > 0x0FFF) V[15] = 0x01;
}
// Font character
void Chip8::op_fx29()
{
    I = RAM[(V[Vx] & 0x000F)*5 + FONT_START];
}
// Binary-coded decimal conversion
void Chip8::op_fx33()
{
    uint_8 x = V[Vx];
    RAM[I + 2] = x % 10;
    uint_8 y = x / 10;
    RAM[I + 1] = y % 10;
    y /= 10;
    RAM[I] = y;
}
// Store memory
void Chip8::op_fx55()
{
    for (uint_8 i = 0; i <= Vx; i++)
    {
        RAM[I + i] = V[i];
    }
}
// Load memory
void Chip8::op_fx65()
{
    for (uint_8 i = 0; i <= Vx; i++)
    {
        V[i] = RAM[I + i];
    }
}

void Chip8::op_null()
{
}
