#ifndef CHIP8_H
#define CHIP8_H



typedef unsigned char uint_8;
typedef unsigned short int uint_16;


/// @brief CHIP-8 interpreter class for interpreting / "emulating" CHIP-8 ROMs.
/// Can be hooked into multiple platforms such as SDL, SFML, OpenGL + GLFW/, etc.
class Chip8
{
private:
    static const uint_16 MEMORY_SIZE = 4096;
    static const uint_16 PROGRAM_START = 0x0200;
    static const uint_8 FONT_START = 0x050;
    static const uint_8 DISPLAY_WIDTH = 64;
    static const uint_8 DISPLAY_HEIGHT = 32;

    uint_16 Stack[16];
    uint_16 SP;             // Stack Pointer
    
    uint_8 RAM[4096];
    uint_8 V[16];           // Variable registers: V0 - VF; VF is flag register

    uint_16 PC;             // Points at current instruction in memory
    uint_16 I;              // Index register - point at locations in memory
    uint_16 Opcode;         // Opcode - stores the current instruction

    // Helpers
    // uint_16 PrevOpcode;     // For debug UI
    uint_16 Nib1;           // First nibble (half byte) N000
    uint_16 Nib2;           // Last nibble 000N
    uint_16 Vx;             // Register Vx, not value in Vx
    uint_16 Vy;             // Register Vy, not value in Vy

    void initialize();

    // Table functions
    void Table0();
    void Table8();
    void TableE();
    void TableF();
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
    
    uint_8 Keys[16];        // HEX keypad for current frame
    // Previous values, used for debugging
    uint_8 PrevKeys[16];    // HEX keys from last frame; used for telling when key is released
    uint_16 PrevStack[16];
    uint_16 PrevSP;             // Stack Pointer
    
    uint_8 PrevRAM[4096];
    uint_8 PrevV[16];           // Variable registers: V0 - VF; VF is flag register

    uint_16 PrevPC;             // Points at current instruction in memory
    uint_16 PrevI;              // Index register - point at locations in memory
    uint_16 PrevOpcode;         // Opcode - stores the current instruction

    // Timers
    uint_8 DelayTimer;
    uint_8 SoundTimer;

    unsigned int Display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    bool DrawFlag;          // Set when draw required

    // Configuration
    // Used for ambiguous instructions, i.e. instructions that change between CHIP-8 and SUPER-CHIP.
    // 0 - CHIP-8   1 - SUPER-CHIP
    bool ConfigShift;
    bool ConfigJumpWOffset;
    // Styling
    unsigned int OnColor;
    unsigned int OffColor;

    Chip8();
    ~Chip8();

    void cycle();           // Runs one emulation cycle
    bool loadROM(char* filePath);
    void storePrevValues();

    // Config
    void setOnColor(unsigned int color);
    void setOffColor(unsigned int color);

    // Immutable getters for debugging
    const uint_16& getStack() const { return *Stack; }
    const uint_16 getSP() const { return SP; }
    const uint_8* getRAM() const { return RAM; }
    const uint_8* getVs() const { return V; }
    const uint_16 getPC() const { return PC; }
    const uint_16 getI() const { return I; }
    const uint_16 getOpcode() const { return Opcode; }
    const uint_16 getOpcodeAt(uint_16 addr) const { return (RAM[addr] << 8) | RAM[addr + 1]; }
    const uint_8& getKeys() const { return *Keys; }
    const uint_8& getPrevKeys() const { return *PrevKeys; }
    const uint_8 getSoundTimer() const { return SoundTimer; }
    const uint_8 getDelayTimer() const { return DelayTimer; }
    const unsigned int& getDisplay() const { return *Display; }
    const char* getOpcodeDescription(uint_16 opcode);

    // Function pointer alias for Opcode FP tables
    typedef void (Chip8::*Chip8Func)();
    // Table size based on highest opcode value in that set
    // e.g. table F has opcode fx65, so we need 0x65 + 1 to index into
    Chip8Func FnTable[0xF + 1];
    Chip8Func FnTable0[0xE + 1];
    Chip8Func FnTable8[0xE + 1];
    Chip8Func FnTableE[0xE + 1];
    Chip8Func FnTableF[0x65 + 1];
};

#endif