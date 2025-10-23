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
    if (argc < 4)
    {
        std::cerr << "CHIP-8 Usage: " << argv[0] << " <ROM File Path> <DisplayScale int> <CycleDelay int>\n\t"
        << "<opt:OnColor 0xAARRGGBB> <opt:OffColor 0xAARRGGBB>\n";
        return -1;
    }
    
    char* fileName = argv[1];
    char displayTitle[128] = "CHIP-8: ";
    strncat(displayTitle, fileName, 118);
    int displayScale = std::stoi(argv[2]);
    int cycleDelay = std::stoi(argv[3]);
    
    // Create emulation / debug window
    Chip8Platform platform(displayTitle, 1920, 1080, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    
    Chip8 chip8;
    
    // Config
    if (argc > 4) chip8.setOnColor(std::stoul(argv[4], nullptr, 16));
    if (argc > 5) chip8.setOffColor(std::stoul(argv[5], nullptr, 16));

    chip8.loadROM(fileName);

    clock_t lastCycleTime = clock();

    // Continue running program
    bool done = false;
    // CHIP-8 cycles paused
    // Emulate next cycle
    bool nextCycle = false;

    while (!done)
    {   
        done = platform.processInput(chip8.Keys, chip8.PrevKeys, platform.debugPause, platform.debugNextCycle);

        clock_t currentTime = clock();
        float deltaTime = (currentTime - lastCycleTime);

        if ((deltaTime > cycleDelay && !platform.debugPause) || (platform.debugPause &&  platform.debugNextCycle && deltaTime > cycleDelay*8))
        {
            lastCycleTime = currentTime;
            chip8.cycle();
        }
        
        platform.debugNextCycle = false;    // This gets set in renderUI
        platform.writeToBuffer(chip8.Display);
        platform.renderUI(chip8);
    }

    return 0;
}