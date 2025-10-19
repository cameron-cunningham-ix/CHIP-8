// CHIP-8 Display Module
#include <stdint.h>
#include <memory.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

// Display
// Each byte is a pixel.
unsigned char Display[64 * 32];

// Test Display
unsigned char TestDisplay[64 * 32] = {0xFF};

void ClearDisplay()
{
    memset(Display, 0x00, 64 * 32);
}