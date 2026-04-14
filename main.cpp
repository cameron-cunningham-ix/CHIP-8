#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
    
    // Continue running program
    bool done = false;
    // CHIP-8 cycles paused
    // Emulate next cycle
    while (!done)
    {   
        done = platform.processInput(chip8.keys, chip8.prevKeys, platform.debugPause, platform.debugNextCycle);

        int cyclesThisFrame = 0;
        if (platform.debugPause)
        {
            // If paused, only run when stepping
            cyclesThisFrame = platform.debugNextCycle ? 1 : 0;
        }
        else
        {
            cyclesThisFrame = platform.cyclesPerFrame;
        }

        for (int i = 0; i < cyclesThisFrame; i++)
        {
            chip8.cycle();
        }
        platform.debugNextCycle = false;
        
        // Decrement timers at 60Hz
        static Uint64 lastTimerUpdate = SDL_GetTicks();
        Uint64 currentTicks = SDL_GetTicks();

        if (currentTicks - lastTimerUpdate >= 16) // Approx 60Hz
        {
            if (chip8.delayTimer > 0)
                chip8.delayTimer--;
            if (chip8.soundTimer > 0)
                chip8.soundTimer--;
            lastTimerUpdate = currentTicks;
        }

        platform.updateAudio(chip8.soundTimer);

        if (chip8.drawFlag)
        {
            platform.writeToBuffer(chip8.display);
        }
        
        platform.renderUI(chip8);
        chip8.drawFlag = false;

        // Load & Save state
        if (platform.saveNewState)
        {
            saveStateChip8 = chip8;
        }
        if (platform.loadSaveState)
        {
            chip8 = saveStateChip8;
            chip8.drawFlag = true;
        }
        
        platform.saveNewState = false;
        platform.loadSaveState = false;
    }

    return 0;
}