#include <iostream>
#include <cstring>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

/// @brief SDL platform for CHIP-8 rendering and input
class Chip8Platform
{
private:
    unsigned int* frameBuffer = nullptr;
    static int done;
    int windowWidth;
    int windowHeight;
    int textureWidth;
    int textureHeight;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    
public:
    // Debug flags
    bool debugPause = false;
    bool debugNextCycle = false;
    bool debugUI = true;
    // Debug Chip8 values
    uint_8 PrevV[16];
    uint_16 PrevPC;
    uint_16 PrevI;
    uint_16 PrevOpcode;

    /// @brief 
    /// @param title Title of SDL window created, appears at top
    /// @param windowWidth SDL window width including UI
    /// @param windowHeight SDL window height including UI
    /// @param textureWidth Width of CHIP-8 texture
    /// @param textureHeight Height of CHIP-8 texture
    Chip8Platform(char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        // SDL Initializations
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize\n";
        }

        float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;

        window = SDL_CreateWindow(title, windowWidth, windowHeight, windowFlags);
        if (!window)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the window\n";
        }
        this->windowWidth = windowWidth;
        this->windowHeight = windowHeight;
        
        renderer = SDL_CreateRenderer(window, NULL);
        SDL_SetRenderVSync(renderer, 1);
        if (!renderer)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the renderer\n";
        }

        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(window);
        
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

        // ImGui Initialization
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = mainScale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
        //io.ConfigDpiScaleFonts = true;        // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
        //io.ConfigDpiScaleViewports = true;    // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);
    }

    ~Chip8Platform()
    {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    /// @brief 
    void renderUI(Chip8 chip8)
    {
        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
        // Set ImGui window to match viewport (SDL window)
        ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));

        ImGui::Begin("UI", NULL, flags);
        // Debug UI
        if (debugUI)
        {
            // Main menu bar
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Menu"))
                {
                    // ImGui::MenuItem("Open", NULL, NULL);
                    ImGui::EndMenu();
    
                }
                ImGui::EndMenu();
            }

            if (!debugPause)
            {
                if (ImGui::Button("Pause Emulation"))
                {
                    debugPause = true;
                }
            } 
            else
            {
                if (ImGui::Button("Resume Emulation"))
                {
                    debugPause = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Next Cycle"))
            {
                debugNextCycle = true;
            }
            // CHIP-8 display
            ImGui::Image((void*)texture, ImVec2(textureWidth*16, textureHeight*16));
            ImGui::SameLine();

            // RAM table config
            ImVec2 ramOuterSize = ImVec2(0.0f, 460.0f);
            // RAM 
            if (ImGui::BeginTable("RAM", 17,
                    (ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                     ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg),
                    ramOuterSize))
            {
                ImGuiListClipper clipper;
                clipper.Begin(256);
                while(clipper.Step())
                {
                    for (uint_16 row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImU32 cellBgColor = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.9f, 0.65f));
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f) ,"0x%02X", row);

                        for (int col = 0; col < 16; col++)
                        {
                            ImGui::TableSetColumnIndex(col + 1);

                            if (chip8.PrevPC == (col + row*16) || chip8.PrevPC == (col + row*16)-1)
                            {
                                // Highlight previous PC
                                cellBgColor = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                                ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f) ,"0x%02X", chip8.getRAM()[col + row*16]);
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                            }
                            else if (chip8.getPC() == (col + row*16) || chip8.getPC() == (col + row*16)-1)
                            {
                                // Highlight current PC
                                cellBgColor = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                                ImGui::Text("0x%02X", chip8.getRAM()[col + row*16]);
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                            }
                            else
                            {
                                ImGui::Text("0x%02X", chip8.getRAM()[col + row*16]);
                            }
                        }
                    }

                }
                ImGui::EndTable();
            }
        
            // PC
            ImGui::Text("Prev PC: %04X", chip8.PrevPC);
            ImGui::Text("PC: %04X", chip8.getPC());
            ImGui::Text("Prev Opcode: %04X", chip8.PrevOpcode);
            ImGui::Text("Curr Opcode: %04X", chip8.getOpcode());

            // Prev Registers table config
            ImVec2 vOuterSize = ImVec2(200.0f, 50.0f);
            // Prev V
            if (ImGui::BeginTable("Prev V", 2,
                    (ImGuiTableFlags_RowBg |
                     ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedSame), vOuterSize))
            {
                ImGui::TableSetupColumn("V");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                for (uint_16 row = 0; row < 16; row++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("V[0x%1X]", row);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", chip8.PrevV[row]);

                }
                ImGui::EndTable();
            }
            ImGui::SameLine();
            // Curr V
            if (ImGui::BeginTable("Curr V", 2,
                    (ImGuiTableFlags_RowBg |
                     ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedSame), vOuterSize))
            {
                ImGui::TableSetupColumn("V");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                for (uint_16 row = 0; row < 16; row++)
                {
                    // ImGui::TableNextRow();
                    // ImU32 cellBgColor = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.9f, 0.65f));
                    // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                    // ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f) ,"0x%02X", row);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("V[0x%X]", row);
                    ImGui::TableSetColumnIndex(1);
                    if (chip8.getVs()[row] != chip8.PrevV[row])
                    {
                        ImU32 cellBgColor = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f) ,"0x%02X", chip8.getVs()[row]);
                    }
                    else
                    {
                        ImGui::Text("0x%02X", chip8.getVs()[row]);
                    }

                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
        // End UI

        char* pix = NULL;
        int pitch = 0;

        SDL_LockTexture(texture, NULL, (void**)&pix, &pitch);
        for (int i = 0, sp = 0, dp = 0; i < textureHeight; i++, dp += textureWidth, sp += pitch)
            memcpy(pix + sp, frameBuffer + dp, textureWidth*sizeof(unsigned int));
        
        SDL_UnlockTexture(texture);
        if (!debugUI)
        {
            SDL_RenderTexture(renderer, texture, NULL, NULL);
        }
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        // Clear buffer
        memset(frameBuffer, 0, textureWidth * textureHeight * sizeof(unsigned int));
        SDL_RenderClear(renderer);
        // SDL_Delay(1);

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
            ImGui_ImplSDL3_ProcessEvent(&e);
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