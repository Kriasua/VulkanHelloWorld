cd /d "%~dp0"
echo Compiling shaders...

glslangValidator.exe -V HelloTriangle.vert -o vert.spv
glslangValidator.exe -V HelloTriangle.frag -o frag.spv

pause