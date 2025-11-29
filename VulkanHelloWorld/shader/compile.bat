cd /d "%~dp0"
echo Compiling shaders...

C:\MyResearch\vulkan\vulkanSDK\Bin\glslangValidator.exe -V HelloTriangle.vert -o vert.spv
C:\MyResearch\vulkan\vulkanSDK\Bin\glslangValidator.exe -V HelloTriangle.frag -o frag.spv

pause