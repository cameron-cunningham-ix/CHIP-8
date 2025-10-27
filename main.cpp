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
    
    clock_t lastCycleTime = clock();

    // Continue running program
    bool done = false;
    // CHIP-8 cycles paused
    // Emulate next cycle
    while (!done)
    {   
        done = platform.processInput(chip8.Keys, chip8.PrevKeys, platform.debugPause, platform.debugNextCycle);

        clock_t currentTime = clock();
        float deltaTime = (currentTime - lastCycleTime);

        if ((deltaTime > chip8.CycleDelay && !platform.debugPause) || (platform.debugPause &&  platform.debugNextCycle && deltaTime > chip8.CycleDelay*8))
        {
            lastCycleTime = currentTime;
            chip8.cycle();
        }
        
        platform.debugNextCycle = false;    // This gets set in renderUI
        if (chip8.DrawFlag)
        {
            platform.writeToBuffer(chip8.Display);
        }
        
        platform.renderUI(chip8);

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