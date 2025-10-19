// CHIP-8 Main Module
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "memory.h"
#include "display.h"

// SDL Globals
int* gFrameBuffer;
SDL_Window* gSDLWindow;
SDL_Renderer* gSDLRenderer;
SDL_Texture* gSDLTexture;
static int gDone;

int gSDLColor = 0xFFFF00FF;

char* fileName = "C:\\Users\\bluej\\SoftDevProjects\\GitHub\\CHIP-8\\roms\\5-quirks.ch8";

// CHIP-8 Configurables
// Used for ambiguous instructions, i.e. instructions that change between CHIP-8 and SUPER-CHIP.
// 0 - CHIP-8   1 - SUPER-CHIP
uint_8 configShift = 0;  // For 8XY6 nad 8XYE
uint_8 configJumpWOffset = 0;   // For BNNN
uint_8 Pause = 0;

int update()
{
    int quit = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_EVENT_QUIT:
                quit = 1;
                break;
            case SDL_EVENT_KEY_DOWN:
            {
                switch (e.key.key)
                {
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    case SDLK_X:
                        keys[0] = 1;
                        break;
                    case SDLK_1:
                        SDL_Log("1 pressed");
                        keys[1] = 1;
                        SDL_Log("%X", keys[1]);
                        break;
                    case SDLK_2:
                        keys[2] = 1;
                        break;
                    case SDLK_3:
                        keys[3] = 1;
                        break;
                    case SDLK_Q:
                        keys[4] = 1;
                        break;
                    case SDLK_W:
                        keys[5] = 1;
                        break;
                    case SDLK_E:
                        keys[6] = 1;
                        break;
                    case SDLK_A:
                        keys[7] = 1;
                        break;
                    case SDLK_S:
                        keys[8] = 1;
                        break;
                    case SDLK_D:
                        keys[9] = 1;
                        break;
                    case SDLK_Z:
                        keys[10] = 1;
                        break;
                    case SDLK_C:
                        keys[11] = 1;
                        break;
                    case SDLK_4:
                        keys[12] = 1;
                        break;
                    case SDLK_R:
                        keys[13] = 1;
                        break;
                    case SDLK_F:
                        keys[14] = 1;
                        break;
                    case SDLK_V:
                        keys[15] = 1;
                        break;
                }
            }
            break;

            case SDL_EVENT_KEY_UP:
            {
                switch (e.key.key)
                {
                    case SDLK_X:
                        keys[0] = 0;
                        break;
                    case SDLK_1:
                        keys[1] = 0;
                        SDL_Log("1 up");
                        break;
                    case SDLK_2:
                        keys[2] = 0;
                        break;
                    case SDLK_3:
                        keys[3] = 0;
                        break;
                    case SDLK_Q:
                        keys[4] = 0;
                        break;
                    case SDLK_W:
                        keys[5] = 0;
                        break;
                    case SDLK_E:
                        keys[6] = 0;
                        break;
                    case SDLK_A:
                        keys[7] = 0;
                        break;
                    case SDLK_S:
                        keys[8] = 0;
                        break;
                    case SDLK_D:
                        keys[9] = 0;
                        break;
                    case SDLK_Z:
                        keys[10] = 0;
                        break;
                    case SDLK_C:
                        keys[11] = 0;
                        break;
                    case SDLK_4:
                        keys[12] = 0;
                        break;
                    case SDLK_R:
                        keys[13] = 0;
                        break;
                    case SDLK_F:
                        keys[14] = 0;
                        break;
                    case SDLK_V:
                        keys[15] = 0;
                        break;
                }
            }
            break;
        }
    }

    char* pix = NULL;
    int pitch = 0;

    SDL_LockTexture(gSDLTexture, NULL, (void**)&pix, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < DISPLAY_HEIGHT; i++, dp += DISPLAY_WIDTH, sp += pitch)
        memcpy(pix + sp, gFrameBuffer + dp, DISPLAY_WIDTH*4);
    
    SDL_UnlockTexture(gSDLTexture);
    SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);
    SDL_RenderPresent(gSDLRenderer);
    SDL_Delay(1);
    return quit;
}

void render()
{
    for (int i = 0, c = 0; i < DISPLAY_HEIGHT; i++)
    {
        for (int j = 0; j < DISPLAY_WIDTH; j++, c++)
        {
            gFrameBuffer[c] = (int)(Display[j + i*64])*gSDLColor | 0xFF000000;
        }
    }
}

void sdlLoop()
{
    if (update())
    {
        gDone = 1;
    }
    else
    {
        render();
    }
}

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        return -1;
    }
    gFrameBuffer = (int*)malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(int));
    gSDLWindow = SDL_CreateWindow("SDL3 Window", DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);
    gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
    gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    // sets texture scale mode to nearest, making it pixel perfect instead of blurry
    SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_NEAREST);
    
    if (!gFrameBuffer || !gSDLWindow || !gSDLRenderer || !gSDLTexture)
    {
        return -1;
    }
    // Scale up window 
    SDL_SetWindowSize(gSDLWindow, 512, 256);

    gDone = 0;

    // Initialize randomness
    srand((unsigned int)time(0));

    // CHIP-8 
    // Format: chip8 file.ch8
    /*if (argc != 2)
    {
        return -1;
    }*/

    FILE *fptr;
    fptr = fopen(fileName, "rb");

    if (fptr == NULL)
    {
        printf("File not opened.");
        return -1;
    }

    // CHIP-8
    InitializeMemory();
    // Write program to RAM
    fread(RAM + 0x200, sizeof(uint_8), 4096 - 0x200, fptr);
    // Close file pointer
    fclose(fptr);

    // Check memory
    // for (int i = 0; i < 200; i++)
    // {
    //     SDL_Log("RAM i: %d - %02X\n", i, RAM[i]);
    // }

    // Main loop - FETCH - DECODE - EXECUTE
    // Used to 
    int InstructionCount = 0;

    // Initialize PC to 0x200 (512) - the start of the program
    PC = 0x200;
    while (!gDone)
    {
        // Read the next 16-bit instruction
        Opcode = (RAM[PC] << 8) | RAM[PC + 1];
        // SDL_Log("%X", Opcode);
        // Increment PC by 2 to be ready for next opcode
        PC += 2;

        // Get Opcode values
        uint_16 nib1 = Opcode & 0xF000;         // First nibble (half byte)
        uint_16 nib2 = Opcode & 0x000F;         // Last nibble
        uint_16 Vx = (Opcode & 0x0F00) >> 8;    // Register x, not value in Vx
        uint_16 Vy = (Opcode & 0x00F0) >> 4;    // Register y, not value in Vy
        uint_8 x = 0;
        switch (nib1)
        {
            // Clear screen or RET
            case 0x0000:
                // Clear screen
                if (Opcode == 0x00E0)
                {
                    SDL_Log("Clear display\n");
                    ClearDisplay();
                } else {    // RET
                    // Pop last addr from Stack
                    SP--;
                    PC = Stack[SP];
                }
                break;
            // Jump - Jump to address NNN
            case 0x1000:
                PC = Opcode & 0x0FFF;
                // SDL_Log("jump to %X\n", PC);
                break;
            // Call addr
            case 0x2000:
                // Push current PC onto stack and inc SP
                Stack[SP] = PC;
                SP++;
                // Jump to NNN
                PC = Opcode & 0x0FFF;
                break;
            // Skip instruction if Vx val == NN
            case 0x3000:
                if (Registers[Vx] == (Opcode & 0x00FF)) PC += 2;
                break;
            // Skip instruction if Vx val != NN
            case 0x4000:
                if (Registers[Vx] != (Opcode & 0x00FF)) PC += 2;
                break;
            // Skip instruction if Vx == Vy
            case 0x5000:
                if (Registers[Vx] == Registers[Vy]) PC += 2;
                break;
            // 6xNN - Set register Vx to NN
            case 0x6000:
                Registers[Vx] = Opcode & 0x00FF;
                // SDL_Log("Set Register %d: %X\n", (Opcode & 0x0F00) >> 8, Registers[(Opcode & 0x0F00) >> 8]);
                break;
            // 7xNN - Add NN to register Vx
            case 0x7000:
                Registers[Vx] += (Opcode & 0x00FF);
                // SDL_Log("Add Register %d: %X\n", (Opcode & 0x0F00) >> 8, Registers[(Opcode & 0x0F00) >> 8]);
                break;
            // Logical / arithmetic ops
            case 0x8000:
                switch (nib2)
                {
                    // Set Vx to Vy
                    case 0x0000:
                        Registers[Vx] = Registers[Vy];
                        break;
                    // Binary OR
                    case 0x0001:
                        Registers[Vx] = Registers[Vx] | Registers[Vy];
                        break;
                    // Binary AND
                    case 0x0002:
                        Registers[Vx] = Registers[Vx] & Registers[Vy];
                        break;
                    // Logical XOR
                    case 0x0003:
                        Registers[Vx] = Registers[Vx] ^ Registers[Vy];
                        break;
                    // Add with overflow detect
                    case 0x0004:
                        x = Registers[Vx];
                        Registers[Vx] = Registers[Vx] + Registers[Vy];
                        // Overflow, set VF to 1
                        if (x > Registers[Vx]) Registers[15] = 0x01;
                        else Registers[15] = 0;
                        break;
                    // Subtract Vx - Vy
                    case 0x0005:
                        // Registers[15] = 0x01;
                        x = Registers[Vx];
                        Registers[Vx] = Registers[Vx] - Registers[Vy];
                        if (Registers[Vx] > x)  Registers[15] = 0x00;   // "Underflowed"
                        else Registers[15] = 0x01;
                        break;
                    // Shift right
                    case 0x0006:
                        if (!configShift)
                        {
                            Registers[Vx] = Registers[Vy];
                        }
                        x = Registers[Vx];
                        Registers[Vx] = x >> 1;
                        Registers[15] = (x & 0x01);
                        break;
                    // Subtract Vy - Vx
                    case 0x0007:
                        x = Registers[Vy];
                        Registers[Vx] = Registers[Vy] - Registers[Vx];
                        if (Registers[Vx] > x)  Registers[15] = 0x00;   // "Underflowed"
                        else Registers[15] = 0x01;
                        break;
                    // Shift left
                    case 0x000E:
                        if (!configShift)
                        {
                            Registers[Vx] = Registers[Vy];
                        }
                        x = Registers[Vx];
                        Registers[Vx] = x << 1;
                        Registers[15] = (x & 0x80) >> 7;
                        break;
                }
                break;
            // Skip instruction if Vx != Vy
            case 0x9000:
                if (Registers[Vx] != Registers[Vy]) PC += 2;
                break;
            // ANNN - Set index register I to NNN
            case 0xA000:
                I = Opcode & 0x0FFF;
                // SDL_Log("Set I: %X - %d\n", I, I);
                break;
            // Jump with offset
            case 0xB000:
                if (!configJumpWOffset)
                {
                    PC = Opcode & 0x0FFF + Registers[0];
                } else {
                    PC = Opcode & 0x0FFF + Registers[Vx];
                }
                break;
            // Random
            case 0xC000:
                x = (uint_8)rand();
                x &= (Opcode & 0x0FF0);
                Registers[Vx] = x;
                break;
            // DXYN - Display N-byte sprite
            case 0xD000:
            {
                // Get registers that are holding the X-Y coords
                uint_8 height = Opcode & 0x000F;     // Height of the sprite to be drawn

                // Get X-Y coords
                uint_16 x0 = Registers[Vx] & 63;
                uint_16 y0 = Registers[Vy] & 31;

                // Set VF to 0
                Registers[15] = 0;
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
                            Registers[15] = 0x01;
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
                break;
            }
            // Skip if key
            case 0xE000:
            {
                // Skip if key corresponding to value in Vx is pressed    
                if (Opcode & 0x00FF == 0x009E)
                {
                    if (keys[Registers[Vx]]) PC += 2;
                }
                // Skip if key corresponding to value in Vx is not pressed
                else if (Opcode & 0x00FF == 0x00A1)
                {
                    if (!keys[Registers[Vx]]) PC += 2;
                }
                break;
            }
            // F cases
            case 0xF000:
            {
                switch (Opcode & 0x00FF)
                {
                    // Set Vx to value of delay timer
                    case 0x0007:
                        Registers[Vx] = DelayTimer;
                        break;
                    // Set delay timer to value in Vx
                    case 0x0015:
                        DelayTimer = Registers[Vx];
                        break;
                    // Set sound timer to value in Vx
                    case 0x0018:
                        SoundTimer = Registers[Vx];
                        break;
                    // Add to index register
                    case 0x001E:
                        I += Registers[Vx];
                        // NOTE: OG COSMAC VIP interpeter did not set VF if I "overflows", but Amiga's did.
                        if (I > 0x0FFF) Registers[15] = 0x01;
                        break;
                    // Get key
                    case 0x000A:
                    {
                        SDL_Log("%X", PC);
                        PC -= 2;
                        for (uint_8 i = 0; i < 16; i++)
                        {
                            if (keys[i] = 1) 
                            {
                                Registers[Vx] = i;
                                PC += 2;
                                break;
                            }
                        }
                        break;
                    }
                    // Font character
                    case 0x0029:
                        I = RAM[(Registers[Vx] & 0x000F)*5 + FONT_START];
                        break;
                    // Binary-coded decimal conversion
                    case 0x0033:
                    {
                        x = Registers[Vx];
                        RAM[I + 2] = x % 10;
                        uint_8 y = x / 10;
                        RAM[I + 1] = y % 10;
                        y /= 10;
                        RAM[I] = y;
                        break;
                    }
                    // Store memory
                    case 0x0055:
                    {
                        for (uint_8 i = 0; i <= Vx; i++)
                        {
                            RAM[I + i] = Registers[i];
                        }
                        break;
                    }
                    // Load memory
                    case 0x0065:
                    {
                        for (uint_8 i = 0; i <= Vx; i++)
                        {
                            Registers[i] = RAM[I + i];
                        }
                        break;
                    }
                }
            }

        }

        // Decrement delay and sound timers
        if (DelayTimer > 0)
            DelayTimer--;
        if (SoundTimer > 0)
            SoundTimer--;

        sdlLoop();
    }
    fclose(fptr);

    SDL_DestroyTexture(gSDLTexture);
    SDL_DestroyRenderer(gSDLRenderer);
    SDL_DestroyWindow(gSDLWindow);
    SDL_Quit();

    return 0;
}