<h1>WIP CHIP-8 Interpreter</h1>

<h2>HISTORY:</h2><br>
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
<li>Wonderful high-level overview of how to create a CHIP-8 interpreter: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#logical-and-arithmetic-instructions </li>
<li>Test suite of ROMs: https://github.com/Timendus/chip8-test-suite </li>
<li>Another CHIP-8 impl. in C++ used as reference, mainly for the function pointer table setup (wanted to test idea out for use in future GB emulator) and SDL Platform: https://austinmorlan.com/posts/chip8_emulator/ </li>
</ul>
