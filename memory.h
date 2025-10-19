// CHIP-8 Memory Management Module
#include <stdint.h>
#include <string.h>

#define MEMORY_SIZE 4096
#define FONT_START 0x050
#define uint_8 unsigned char
#define uint_16 unsigned short int

// RAM
uint_8 RAM[MEMORY_SIZE];

// Stack
uint_16 Stack[16];

// Stack Pointer
uint_16 SP;

// Program Counter - points at the current instruction in memory
uint_16 PC;

// Index register - used to point at locations in memory
uint_16 I;

// Opcode - stores the current instruction
uint_16 Opcode;

// Variable registers
// V0 - VF; VF is also used as a flag register
uint_8 Registers[16];

// HEX keypad
uint_8 keys[16];

// Timers
uint_8 DelayTimer;
uint_8 SoundTimer;

void InitializeMemory()
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
    // Zero out registers and stack
    for (uint_16 i = 0; i < 16; i++)
    {
        Registers[i] = 0x00;
        Stack[i] = 0;
        keys[i] = 0;
    }
    SP = 0;
    PC = 0;
    I = 0;
    Opcode = 0;
    DelayTimer = 60;
    SoundTimer = 60;
}