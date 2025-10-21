#include <iostream>
#include <cstring>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

/// @brief SDL platform for CHIP-8 rendering and input
class Chip8Platform
{
private:
    unsigned int* frameBuffer = nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    static int done;
    int textureWidth;
    int textureHeight;

public:

    /// @brief 
    /// @param title Title of SDL window created, appears at top
    /// @param windowWidth SDL window width
    /// @param windowHeight SDL window height
    /// @param textureWidth Width of texture drawn to window
    /// @param textureHeight Height of texture drawn to window
    Chip8Platform(char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize\n";
        }

        window = SDL_CreateWindow(title, windowWidth, windowHeight, 0);
        if (!window)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the window\n";
        }
        
        renderer = SDL_CreateRenderer(window, NULL);
        if (!renderer)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the renderer\n";
        }
        
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
        if (!texture)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the texture\n";
        }
        this->textureWidth = textureWidth;
        this->textureHeight = textureHeight;
        // Sets texture scale mode to nearest, making it pixel perfect instead of blurry
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        // Allocate framebuffer
        frameBuffer = (unsigned int*)calloc(textureWidth * textureHeight, sizeof(unsigned int));
        if (!frameBuffer)
        {
            SDL_GetError();
            std::cerr << "SDL failed to allocate framebuffer\n";
        }

        SDL_Log("SDL Initialized\n");
    }

    ~Chip8Platform()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    /// @brief 
    void render()
    {
        char* pix = NULL;
        int pitch = 0;

        SDL_LockTexture(texture, NULL, (void**)&pix, &pitch);
        for (int i = 0, sp = 0, dp = 0; i < textureHeight; i++, dp += textureWidth, sp += pitch)
            memcpy(pix + sp, frameBuffer + dp, textureWidth*sizeof(unsigned int));
        
        SDL_UnlockTexture(texture);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    void writeToBuffer(unsigned int* display)
    {
        for (int i = 0, c = 0; i < textureHeight; i++)
        {
            for (int j = 0; j < textureWidth; j++, c++)
            {
                frameBuffer[c] = (unsigned int)(display[j + i*textureWidth] | 0xFF000000);
            }
        }
    }

    bool processInput(unsigned char* keys, unsigned char* prevKeys, bool& pause, bool& nextCycle)
    {
        // Before processing this frame/cycle, set PrevKeys to Keys
        for (int i = 0; i < 16; i++)
        {
            prevKeys[i] = keys[i];
        }

        bool quit = false;
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            switch(e.type)
            {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                {
                    switch (e.key.scancode)
                    {
                        case SDL_SCANCODE_ESCAPE:
                            quit = true;
                            break;
                        // Debug
                        case SDL_SCANCODE_RIGHT:
                            nextCycle = true;
                            break;
                        // Hex Keypad
                        case SDL_SCANCODE_X:
                            keys[0] = 1;
                            break;
                        case SDL_SCANCODE_1:
                            keys[1] = 1;
                            break;
                        case SDL_SCANCODE_2:
                            keys[2] = 1;
                            break;
                        case SDL_SCANCODE_3:
                            keys[3] = 1;
                            break;
                        case SDL_SCANCODE_Q:
                            keys[4] = 1;
                            break;
                        case SDL_SCANCODE_W:
                            keys[5] = 1;
                            break;
                        case SDL_SCANCODE_E:
                            keys[6] = 1;
                            break;
                        case SDL_SCANCODE_A:
                            keys[7] = 1;
                            break;
                        case SDL_SCANCODE_S:
                            keys[8] = 1;
                            break;
                        case SDL_SCANCODE_D:
                            keys[9] = 1;
                            break;
                        case SDL_SCANCODE_Z:
                            keys[10] = 1;
                            break;
                        case SDL_SCANCODE_C:
                            keys[11] = 1;
                            break;
                        case SDL_SCANCODE_4:
                            keys[12] = 1;
                            break;
                        case SDL_SCANCODE_R:
                            keys[13] = 1;
                            break;
                        case SDL_SCANCODE_F:
                            keys[14] = 1;
                            break;
                        case SDL_SCANCODE_V:
                            keys[15] = 1;
                            break;
                    }
                }
                break;

                case SDL_EVENT_KEY_UP:
                {
                    switch (e.key.scancode)
                    {
                        // Debug
                        case SDL_SCANCODE_SPACE:
                            pause = !pause;
                            break;
                        case SDL_SCANCODE_RIGHT:
                            nextCycle = false;
                            break;
                        // Hex Keypad
                        case SDL_SCANCODE_X:
                            keys[0] = 0;
                            break;
                        case SDL_SCANCODE_1:
                            keys[1] = 0;
                            break;
                        case SDL_SCANCODE_2:
                            keys[2] = 0;
                            break;
                        case SDL_SCANCODE_3:
                            keys[3] = 0;
                            break;
                        case SDL_SCANCODE_Q:
                            keys[4] = 0;
                            break;
                        case SDL_SCANCODE_W:
                            keys[5] = 0;
                            break;
                        case SDL_SCANCODE_E:
                            keys[6] = 0;
                            break;
                        case SDL_SCANCODE_A:
                            keys[7] = 0;
                            break;
                        case SDL_SCANCODE_S:
                            keys[8] = 0;
                            break;
                        case SDL_SCANCODE_D:
                            keys[9] = 0;
                            break;
                        case SDL_SCANCODE_Z:
                            keys[10] = 0;
                            break;
                        case SDL_SCANCODE_C:
                            keys[11] = 0;
                            break;
                        case SDL_SCANCODE_4:
                            keys[12] = 0;
                            break;
                        case SDL_SCANCODE_R:
                            keys[13] = 0;
                            break;
                        case SDL_SCANCODE_F:
                            keys[14] = 0;
                            break;
                        case SDL_SCANCODE_V:
                            keys[15] = 0;
                            break;
                    }
                }
                break;
            }
        }
        
        return quit;
    }
};