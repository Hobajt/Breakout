# Breakout

Clone of the game Breakout, inspired by <a href="https://learnopengl.com/In-Practice/2D-Game/Breakout">this tutorial</a> from learnopengl.com.

**3rd party libraries:**
* GLFW
* Glad
* glm
* FreeType
* <a href="https://github.com/mackron/miniaudio">miniaudio</a>

Use `git clone --recursive` to also download GLFW & FreeType submodules.

In order to properly load all game resources, program's working directory needs to be set to repository's root dir.

For running in Visual Studio:
- Right click *CMakeLists.txt** > Add Debug Configuration > add entry "currentDir": "${workspaceRoot}"
- Or manually edit *.vs/launch.vs.json*
