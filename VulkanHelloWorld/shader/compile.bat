cd /d "%~dp0"
echo Compiling shaders...

glslangValidator.exe -V vert.vert -o vert.spv
glslangValidator.exe -V frag.frag -o frag.spv

pause