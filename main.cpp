#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <time.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "Chip8.h"
#include "Chip8Platform.h"

const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;

int main(int argc, char *argv[])
{
    char displayTitle[128] = "CHIP-8: ";
    
    // Create emulation / debug window
    Chip8Platform platform(displayTitle, 1280, 720, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    
    // Running CHIP-8
    Chip8 chip8;
    // CHIP-8 to load for save state
    Chip8 saveStateChip8;
    
    clock_t lastFrameTime = clock();

    double frameTime = 1.0 / platform.cyclesPerFrame;

    // Continue running program
    bool done = false;
    // CHIP-8 cycles paused
    // Emulate next cycle
    while (!done)
    {   
        frameTime = 1.0 / platform.cyclesPerFrame;

        done = platform.processInput(chip8.Keys, chip8.PrevKeys, platform.debugPause, platform.debugNextCycle);

        double deltaTime = (clock() - lastFrameTime);

        while ((deltaTime < frameTime && !platform.debugPause) || (platform.debugPause &&  platform.debugNextCycle))
        {
            // lastFrameTime = currentTime;
            deltaTime = (clock() - lastFrameTime);
            chip8.cycle();
            platform.debugNextCycle = false;    // This gets set in renderUI
        }
        
        // platform.debugNextCycle = false;    // This gets set in renderUI
        if (chip8.DrawFlag)
        {
            platform.writeToBuffer(chip8.Display);
        }
        
        platform.renderUI(chip8);
        chip8.DrawFlag = false;

        lastFrameTime = clock();

        // Load & Save state
        if (platform.saveNewState)
        {
            saveStateChip8 = chip8;
        }
        if (platform.loadSaveState)
        {
            chip8 = saveStateChip8;
            chip8.DrawFlag = true;
        }
        
        platform.saveNewState = false;
        platform.loadSaveState = false;
    }

    return 0;
}