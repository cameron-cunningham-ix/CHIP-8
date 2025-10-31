<h1>CHIP-8 Interpreter</h1>
<br>
<img src="pictures/UI_v0_75.png" style="max-width:800px;"> <br>
A CHIP-8 interpreter / "emulator" written in C++

<h2>Usage:</h2>
CHIP8.exe <br>

<h2>HISTORY:</h2><br>
<b>v0.76</b> - Added Reset ROM button <br>
- Added target frame rate slider <br>
- Fixed bug of not being able to load save state <br>
<br>
<b>v0.75</b> - Removed timer decrement from Chip8 cycle function and placed them in main, always running at ~60Hz. This is the correct implementation; some games do use the timers to run their games at a consistent FPS regardless of number of cycles run. <br>
- Added checks for OOB accesses in function table decoders. <br>
- Removed CycleDelay from Chip8 and replaced it with cyclesPerFrame in Chip8Platform; changed main to correspond. Emulation now can run set number of cycles per frame, instead of running max one cycle per frame. <br>
- Replaced default ImGui font with Roboto. <br>
<br>
<b>v0.74</b> - Removed command line args; you now run the .exe and set up everything in program <br>
- Added Open ROM button to UI, uses NativeFileDialog-Extended library to open file dialog for picking ROM to run <br>
- Added Settings button to UI with following options: <br>
    - Cycle Delay: 0 to 128 <br>
    - Shift Vy <br>
    - Jump W/ Offset <br>
    - Display sizes: 1x to 16x <br>
    - On & off colors <br>
- Added vec4ToRGBA function for color conversion <br>
- Minor changes to layout and initialization <br>
<br>
<b>v0.73</b> - Removed vendored folder and SDL and ImGui submodules; CMake build now takes care of fetching the content <br>
- Included NativeFileDialog-Extended, setting up for being able to open ROM files while running program <br>
<br>
<b>v0.72</b> - Removed roms folder for cleaner repo and licensing <br>
- Removed VSync from renderUI, as it was slowing emulation down to screen refresh rate <br>
- Added simple load and save state <br>
- Minor UI layout changes <br>
<br>
<b>v0.71</b> - Added some basic window scaling layout <br>
<br>
<b>v0.7</b> - Added more information to debug UI (previous & current SP, stack) <br>
- Added some text labels to UI <br>
<br>
<b>v0.6</b> - Included ImGui into project. Using the SDL3Renderer backend for ease <br>
- Added initial parts of ImGui debugging window <br>
- Added fields for previous cycle's values for things like the stack, memory, and registers to Chip8, also for debugging <br>
- Added .gitmodules for ease of grabbing SDL and ImGui <br>
<br>
<b>v0.5</b> - Big jump from v0.01, but justified as the interpreter is now fully working! (I believe. I haven't had any tests or games break it so far)<br> - Created Chip8Platform class, taking inspiration from https://austinmorlan.com/posts/chip8_emulator/ <br> - Added some Doxygen comments<br> - Added color configuration<br> - Changed configuration to run off of input arguments <br> - Added very basic pause and step through functionality<br> - Included 2 more ROMs, 1D cellular automata and breakout<br> - Renamed the Tetris file since for some reason the previous name crashed the program on startup. Certainly something with the displayTitle concat to fix<br>
<br>
The next goal for v1.0 is to do step 2, create a debug window that displays the current opcode, registers and values, stack, and memory with ImGUI.<br>
<br>
<b>v0.01</b> - First commit! Doing this now because the interpreter is close to fully working in C, but I can't determine why
keypresses are not triggering the right behavior. I need a better way of debugging and to do that, I'm going to: <br>

1. Convert to C++ and encapsulate everything CHIP8 related in its own class for easier use <br>
2. Create a debugging interface with ImGUI

<h2>Acknowledgements:</h2>
<ul>
<li>NativeFileDialog-Extended library: https://github.com/btzy/nativefiledialog-extended
<li>Wonderful high-level overview of how to create a CHIP-8 interpreter: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#logical-and-arithmetic-instructions </li>
<li>Test suite of ROMs: https://github.com/Timendus/chip8-test-suite </li>
<li>Another CHIP-8 impl. in C++ used as reference, mainly for the function pointer table setup (wanted to test idea out for use in future GB emulator) and SDL Platform: https://austinmorlan.com/posts/chip8_emulator/ </li>
</ul>
