#include <iostream>
#include <cstring>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "nfd.h"

/// @brief SDL + ImGui platform for CHIP-8 rendering and input
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
    // UI flags
    bool m_ShowEmulationSettings = false;
    // UI proportions
    ImVec2 debugWindowProportion = ImVec2(0.25f, 0.45f);
    ImVec2 displayWindowProportion = ImVec2(0.5f, 0.5f);
    ImVec2 opcodeWindowProportion = ImVec2(0.25f, 0.5f);
    ImVec2 ramWindowProportion = ImVec2(0.5f, 0.5f);
    ImVec2 registersWindowProportion = ImVec2(0.5f, 0.5f);

    // Debug flags
    bool debugPause = false;
    bool debugNextCycle = false;
    bool debugUI = true;
    bool saveNewState = false;
    bool loadSaveState = false;
    // Platform
    float mainScale;
    int textureScale = 8;
    ImVec4 onColorRef = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 offColorRef = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

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

        mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;

        window = SDL_CreateWindow(title, (int)windowWidth*mainScale, (int)windowHeight*mainScale, windowFlags);
        if (!window)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the window\n";
        }
        this->windowWidth = windowWidth*mainScale;
        this->windowHeight = windowHeight*mainScale;
        
        renderer = SDL_CreateRenderer(window, NULL);
        if (!renderer)
        {
            SDL_GetError();
            std::cerr << "SDL failed to initialize the renderer\n";
        }

        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(window);
        
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
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
        this->textureScale = textureScale;
        SDL_Log("SDL Initialized\n");

        // ImGui Initialization
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

        // io.DisplaySize.x = (float)(windowWidth*mainScale);
        // io.DisplaySize.y = (float)(windowHeight*mainScale);

        ImFontConfig fontConfig;
        fontConfig.SizePixels = 14.0f * mainScale;
        io.Fonts->AddFontDefault(&fontConfig);
        // io.Fonts->Build();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = mainScale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
        //io.ConfigDpiScaleFonts = true;        // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
        //io.ConfigDpiScaleViewports = true;    // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        // Initialize NFD
        if (NFD_Init() != NFD_OKAY)
        {
            SDL_Log("Native File Dialog failed to initialize\n");
        }
    }

    ~Chip8Platform()
    {
        NFD_Quit();
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();\
        ImGui::DestroyContext();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    /// @brief Draw the display / emulation ImGui window
    void drawDisplayWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(windowWidth * displayWindowProportion.x, 0), ImGuiCond_None);
        ImGui::SetNextWindowSize(ImVec2(windowWidth*displayWindowProportion.x,
            windowHeight*displayWindowProportion.y));
        ImGui::Begin("Display", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::Image((void*)texture, ImVec2(textureWidth*textureScale,
            textureHeight*textureScale));
        ImGui::End();
    }

    void drawDebugWindow(Chip8 &chip8)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_None);
        ImGui::SetNextWindowSize(ImVec2(windowWidth * debugWindowProportion.x, windowHeight * debugWindowProportion.y));
        ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button("Open ROM"))
        {
            // File dialog open
            nfdu8char_t *outPath;
            nfdu8filteritem_t filters[1] = { { "ROMs", "ch8" }};
            nfdopendialogu8args_t args = {0};
            args.filterList = filters;
            args.filterCount = 1;
            nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
            if (result == NFD_OKAY)
            {
                puts("Success!");
                puts(outPath);
                if (chip8.loadROM(outPath))
                {
                    puts("ROM loaded sucessfully");
                }
                NFD_FreePathU8(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                puts("User pressed cancel.");
            }
            else 
            {
                printf("Error: %s\n", NFD_GetError());
            }
        }
        // ImGui::SameLine();
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
        // ImGui::SameLine();
        if (ImGui::Button("Next Cycle"))
        {
            debugNextCycle = true;
        }
        // ImGui::SameLine();
        if (ImGui::Button("Save State"))
        {
            saveNewState = true;
        }
        // ImGui::SameLine();
        if (ImGui::Button("Load Save State"))
        {
            loadSaveState = true;
        }
        // ImGui::SameLine();
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::CollapsingHeader("Emulation", ImGuiTreeNodeFlags_None))                    
            {
                ImGui::SliderInt("Cycle Delay", &chip8.CycleDelay, 0, 128);
                ImGui::SetItemTooltip("Min delay between cycles (in ms)");
                ImGui::Checkbox("Shift Vy", &chip8.ConfigShift);
                ImGui::SetItemTooltip("Off - CHIP-8 functionality\nOn - CHIP-48 & SCHIP functionality");
                ImGui::Checkbox("Jump W/ Offset", &chip8.ConfigJumpWOffset);
                ImGui::SetItemTooltip("Off - CHIP-8 functionality\nOn - CHIP-48 & SCHIP functionality");
            }
            if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_None))
            {
                // ImGui::Text("Display size:");
                // ImGui::SameLine();
                // if (ImGui::Button("1x")) textureScale = 1;
                // ImGui::SameLine();
                // if (ImGui::Button("2x")) textureScale = 2;
                // ImGui::SameLine();
                // if (ImGui::Button("4x")) textureScale = 4;
                // ImGui::SameLine();
                // if (ImGui::Button("8x")) textureScale = 8;
                // ImGui::SameLine();
                // if (ImGui::Button("16x")) textureScale = 16;

                ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview;
                if (!debugPause) colorFlags |= ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop;
                ImGui::SetItemTooltip("NOTE: Set color only when emulation is paused");
                ImGui::ColorPicker4("On Color", &onColorRef.x, colorFlags);
                ImGui::SetItemTooltip("NOTE: Set color only when emulation is paused");
                ImGui::ColorPicker4("Off Color", &offColorRef.x, colorFlags);

            }
            ImGui::EndMenu();
        }
        // ImGui::PopItemWidth();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "V0.74");
        ImGui::End();
    }

    void drawOpcodeWindow(Chip8 chip8)
    {
        ImGui::SetNextWindowPos(ImVec2(windowWidth * debugWindowProportion.x + 5, 5), ImGuiCond_None);
        ImGui::SetNextWindowSize(ImVec2(windowWidth * opcodeWindowProportion.x - 5,
            windowHeight * opcodeWindowProportion.y - 5));
        ImGui::Begin("Opcode", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
         // Prev values
        ImGui::BeginGroup();
        {
            ImGui::Text("Prev PC: %04X", chip8.PrevPC);
            ImGui::Text("Prev I: %04X", chip8.PrevI);
            ImGui::Text("Prev SP: %04X", chip8.PrevSP);
            ImGui::Text("Prev Opcode: %04X", chip8.PrevOpcode);
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        // Curr values
        ImGui::BeginGroup();
        {
            ImGui::Text("Curr PC: %04X", chip8.getPC());
            ImGui::Text("Curr I: %04X", chip8.getI());
            ImGui::Text("Curr SP: %04X", chip8.getSP());
            ImGui::Text("Curr Opcode: %04X", chip8.getOpcode());
            ImGui::EndGroup();
        }
        ImGui::End();
    }

    void drawRegistersWindow(Chip8 chip8)
    {
        ImGui::SetNextWindowPos(ImVec2(windowWidth * registersWindowProportion.x, windowHeight * registersWindowProportion.y), ImGuiCond_None);
        ImGui::SetNextWindowSize(ImVec2(windowWidth/2, windowHeight/2));
        ImGui::Begin("Registers", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
       

        // Registers table config
        ImVec2 vOuterSize = ImVec2(110.0f, 50.0f);
        // Prev V
        ImGui::BeginGroup();
        {
            ImGui::Text("Prev Registers");
            if (ImGui::BeginTable("Prev V", 2,
                    (ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedSame), vOuterSize))
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
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        // Curr V
        ImGui::BeginGroup();
        {
            ImGui::Text("Curr Registers");
            if (ImGui::BeginTable("Curr V", 2,
                    (ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedSame), vOuterSize))
            {
                ImGui::TableSetupColumn("V");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                for (uint_16 row = 0; row < 16; row++)
                {
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
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        ImVec2 stackOuterSize = ImVec2(140.0f, 50.0f);
        // Prev Stack
        ImGui::BeginGroup();
        {
            ImGui::Text("Prev Stack");
            if (ImGui::BeginTable("Prev Stack", 2,
                    (ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedSame), stackOuterSize))
            {
                ImGui::TableSetupColumn("Stack");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                for (uint_16 row = 0; row < 16; row++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Stack[0x%1X]", row);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", chip8.PrevStack[row]);

                }
                ImGui::EndTable();
            }
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        // Curr Stack
        ImGui::BeginGroup();
        {
            ImGui::Text("Curr Stack");
            if (ImGui::BeginTable("Curr Stack", 2,
                    (ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedSame), stackOuterSize))
            {
                ImGui::TableSetupColumn("Stack");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                for (uint_16 row = 0; row < 16; row++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Stack[0x%X]", row);
                    ImGui::TableSetColumnIndex(1);
                    if (chip8.getStack()[row] != chip8.PrevStack[row])
                    {
                        ImU32 cellBgColor = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor);
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f) ,"0x%02X", chip8.getStack()[row]);
                    }
                    else
                    {
                        ImGui::Text("0x%02X", chip8.getStack()[row]);
                    }

                }
                ImGui::EndTable();
                ImGui::EndGroup();
            }
        }
        ImGui::EndGroup();
        ImGui::End();
    }

    void drawRAMWindow(Chip8 chip8)
    {
        // RAM table config
        ImGui::SetNextWindowPos(ImVec2(0, windowHeight * ramWindowProportion.y), ImGuiCond_None);
        ImVec2 ramOuterSize = ImVec2(windowWidth * ramWindowProportion.x, windowHeight * ramWindowProportion.y);
        ImGui::SetNextWindowSize(ramOuterSize);
        // RAM group - keeps "RAM" and the actual table inline vertically
        ImGui::Begin("RAM", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::BeginTable("RAM", 17,
                (ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | 
                ImGuiTableFlags_SizingFixedFit),
                ramOuterSize))
        {

            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthStretch);
            for (int col = 0; col < 16; col++)
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            }
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
                        // ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
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
        ImGui::End();
    }

    /// @brief Render the UI and emulation display
    void renderUI(Chip8 &chip8)
    {
        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        // Set ImGui window to match viewport (SDL window)
        int currWindowWidth, currWindowHeight;
        SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
        // ImGui::SetNextWindowSize(ImVec2(currWindowWidth, currWindowHeight));
        // ImVec2 availSize = ImVec2((float)currWindowWidth, (float)currWindowHeight);

        // Debug UI
        if (debugUI)
        {
            drawDebugWindow(chip8);
            
            // CHIP-8 display
            drawDisplayWindow();
            
            drawRegistersWindow(chip8);
            
            drawRAMWindow(chip8);

            drawOpcodeWindow(chip8);
            
        }

        chip8.setOnColor(vec4ToRGBA(onColorRef));
        chip8.setOffColor(vec4ToRGBA(offColorRef));

        // Copy data from frameBuffer into texture
        if (chip8.DrawFlag)
        {
            char* pix = NULL;
            int pitch = 0;
    
            SDL_LockTexture(texture, NULL, (void**)&pix, &pitch);
            for (int i = 0, sp = 0, dp = 0; i < textureHeight; i++, dp += textureWidth, sp += pitch)
                memcpy(pix + sp, frameBuffer + dp, textureWidth*sizeof(unsigned int));
            
            SDL_UnlockTexture(texture);
        }
        // If not in debug mode, just render emulation
        if (!debugUI)
        {
            SDL_RenderTexture(renderer, texture, NULL, NULL);
        }
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        // Clear renderer for next frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);
    }

    /// @brief Write data to frameBuffer
    /// @param display 
    void writeToBuffer(unsigned int* display)
    {
        for (int i = 0, c = 0; i < textureHeight; i++)
        {
            for (int j = 0; j < textureWidth; j++, c++)
            {
                frameBuffer[c] = display[j + i*textureWidth];
            }
        }
    }

    unsigned int vec4ToRGBA(ImVec4 vec4)
    {
        unsigned int r = vec4.x * 255;
        unsigned int g = vec4.y * 255;
        unsigned int b = vec4.z * 255;
        unsigned int a = vec4.w * 255;
        unsigned int val = (r << 24) | g << 16 | b << 8 | a;
        return val;
    }

    /// @brief Process input events for CHIP-8
    /// @param keys 
    /// @param prevKeys 
    /// @param pause 
    /// @param nextCycle 
    /// @return True when quit 
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