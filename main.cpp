// CHIP-8 Main Module
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "Chip8.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

// SDL Globals
int* gFrameBuffer;
SDL_Window* gSDLWindow;
SDL_Renderer* gSDLRenderer;
SDL_Texture* gSDLTexture;
static int gDone;

int gSDLColor = 0xFFFF00FF;

char* fileName = "C:\\Users\\bluej\\SoftDevProjects\\GitHub\\CHIP-8\\roms\\3-corax+.ch8";

// CHIP-8 Configurables
// Used for ambiguous instructions, i.e. instructions that change between CHIP-8 and SUPER-CHIP.
// 0 - CHIP-8   1 - SUPER-CHIP
uint_8 configShift = 0;  // For 8XY6 nad 8XYE
uint_8 configJumpWOffset = 0;   // For BNNN
uint_8 Pause = 0;
int DISPLAY_WIDTH = 64;
int DISPLAY_HEIGHT = 32;

int processInput(uint_8* keys)
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

    
    return quit;
}

void update()
{
    char* pix = NULL;
    int pitch = 0;

    SDL_LockTexture(gSDLTexture, NULL, (void**)&pix, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < DISPLAY_HEIGHT; i++, dp += DISPLAY_WIDTH, sp += pitch)
        memcpy(pix + sp, gFrameBuffer + dp, DISPLAY_WIDTH*4);
    
    SDL_UnlockTexture(gSDLTexture);
    SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);
    SDL_RenderPresent(gSDLRenderer);
    SDL_Delay(1);
}

void render(uint_8* display)
{
    for (int i = 0, c = 0; i < DISPLAY_HEIGHT; i++)
    {
        for (int j = 0; j < DISPLAY_WIDTH; j++, c++)
        {
            gFrameBuffer[c] = (int)(display[j + i*64])*gSDLColor | 0xFF000000;
        }
    }
}

void sdlLoop(Chip8 chip8)
{
    if (processInput(chip8.Keys))
    {
        gDone = 1;
    }
    else
    {
        update();
        render(chip8.Display);
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


    // CHIP-8 
    // Format: chip8 file.ch8
    /*if (argc != 2)
    {
        return -1;
    }*/

    Chip8 chip8;
    chip8.loadROM(fileName);

    // Check memory
    // for (int i = 0; i < 200; i++)
    // {
    //     SDL_Log("RAM i: %d - %02X\n", i, RAM[i]);
    // }

    // Main loop - FETCH - DECODE - EXECUTE
    // Used to 
    int InstructionCount = 0;

    while (!gDone)
    {   
        sdlLoop(chip8);
        chip8.cycle();
    }

    SDL_DestroyTexture(gSDLTexture);
    SDL_DestroyRenderer(gSDLRenderer);
    SDL_DestroyWindow(gSDLWindow);
    SDL_Quit();

    return 0;
}