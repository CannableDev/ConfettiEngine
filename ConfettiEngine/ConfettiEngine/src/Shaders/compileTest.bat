:: Check out libshaderc (https://github.com/google/shaderc) in vulkanSDK later for in-program compiling

::C:/VulkanSDK/1.2.189.2/Bin32/glslc.exe testTriangle_vert.vert -o testTriangle_vert.spv
::C:/VulkanSDK/1.2.189.2/Bin32/glslc.exe testTriangle_frag.frag -o testTriangle_frag.spv

for %%i in (*.vert) do C:/VulkanSDK/1.2.189.2/Bin32/glslc.exe %%i -o %%i.spv
for %%i in (*.geom) do C:/VulkanSDK/1.2.189.2/Bin32/glslc.exe %%i -o %%i.spv
for %%i in (*.frag) do C:/VulkanSDK/1.2.189.2/Bin32/glslc.exe %%i -o %%i.spv

pause