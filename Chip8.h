#ifndef CHIP8_H
#define CHIP8_H

typedef unsigned char uint_8;
typedef unsigned short int uint_16;

/// @brief CHIP-8 interpreter class for interpreting / "emulating" CHIP-8 ROMs.
/// Can be hooked into multiple platforms such as SDL, OpenGL + GLFW/, etc.
class Chip8
{
private:
    static const uint_16 MEMORY_SIZE = 4096;
    static const uint_16 PROGRAM_START = 0x0200;
    static const uint_8 FONT_START = 0x050;
    static const uint_8 DISPLAY_WIDTH = 64;
    static const uint_8 DISPLAY_HEIGHT = 32;

    uint_16 stack[16];
    uint_16 sp;             // Stack Pointer
    
    uint_8 ram[4096];
    uint_8 v[16];           // Variable registers: V0 - VF; VF is flag register

    uint_16 pc;             // Points at current instruction in memory
    uint_16 idx;              // Index register - point at locations in memory
    uint_16 opcode;         // Opcode - stores the current instruction

    // Helpers
    // uint_16 prevOpcode;     // For debug UI
    uint_16 nib1;           // First nibble (half byte) N000
    uint_16 nib2;           // Last nibble 000N
    uint_16 vx;             // Register vx, not value in vx
    uint_16 vy;             // Register vy, not value in vy

    void initialize();

    // Table functions
    void table0();
    void table8();
    void tableE();
    void tableF();
    // Opcode functions
    void op_00e0();
    void op_00ee();
    void op_1nnn();
    void op_2nnn();
    void op_3xnn();
    void op_4xnn();
    void op_5xy0();
    void op_6xnn();
    void op_7xnn();
    void op_8xy0();
    void op_8xy1();
    void op_8xy2();
    void op_8xy3();
    void op_8xy4();
    void op_8xy5();
    void op_8xy6();
    void op_8xy7();
    void op_8xyE();
    void op_9xy0();
    void op_annn();
    void op_bnnn();
    void op_cxnn();
    void op_dxyn();
    void op_ex9e();
    void op_exa1();
    void op_fx07();
    void op_fx0a();
    void op_fx15();
    void op_fx18();
    void op_fx1e();
    void op_fx29();
    void op_fx33();
    void op_fx55();
    void op_fx65();
    void op_null();
    
public:
    
    uint_8 keys[16];        // HEX keypad for current frame
    uint_8 prevKeys[16];    // HEX keys from last frame; used for telling when key is released
    
    // Previous values, used for debugging
    uint_16 prevStack[16];
    uint_16 prevSP;             // Stack Pointer
    uint_8 prevV[16];           // Variable registers: V0 - VF; VF is flag register
    uint_16 prevPC;             // Points at current instruction in memory
    uint_16 prevIdx;              // Index register - point at locations in memory
    uint_16 prevOpcode;         // Opcode - stores the current instruction

    // Timers
    uint_8 delayTimer;
    uint_8 soundTimer;

    unsigned int display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    bool drawFlag;          // Set when draw required

    // Configuration
    // Used for ambiguous instructions, i.e. instructions that change between CHIP-8 and SUPER-CHIP.
    // 0 - CHIP-8   1 - SUPER-CHIP
    // int CycleDelay = 1;
    bool configShift;
    bool configJumpWOffset;
    // Styling
    unsigned int onColor;
    unsigned int offColor;

    Chip8();
    ~Chip8();

    void cycle();           // Runs one emulation cycle
    bool loadROM(char* filePath);
    void storePrevValues();

    // Config
    void setOnColor(unsigned int color);
    void setOffColor(unsigned int color);

    // Immutable getters for debugging
    const uint_16* getStack() const { return stack; }
    const uint_16 getSP() const { return sp; }
    const uint_8* getRAM() const { return ram; }
    const uint_8* getVs() const { return v; }
    const uint_16 getPC() const { return pc; }
    const uint_16 getI() const { return idx; }
    const uint_16 getOpcode() const { return opcode; }
    const uint_16 getOpcodeAt(uint_16 addr) const { return (ram[addr] << 8) | ram[addr + 1]; }
    const uint_8* getKeys() const { return keys; }
    const uint_8* getPrevKeys() const { return prevKeys; }
    const uint_8 getSoundTimer() const { return soundTimer; }
    const uint_8 getDelayTimer() const { return delayTimer; }
    const unsigned int* getDisplay() const { return display; }
    const char* getOpcodeDescription(uint_16 opcode);

    // Function pointer alias for Opcode FP tables
    typedef void (Chip8::*Chip8Func)();
    // Table size based on highest opcode value in that set
    // e.g. table F has opcode fx65, so we need 0x65 + 1 to index into
    Chip8Func fnTable[0xF + 1];
    Chip8Func fnTable0[0xE + 1];
    Chip8Func fnTable8[0xE + 1];
    Chip8Func fnTableE[0xE + 1];
    Chip8Func fnTableF[0x65 + 1];
};

#endif