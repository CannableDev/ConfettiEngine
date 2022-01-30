#include "Shader.h"

std::vector<char> Shader::read(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("ERROR: Failed to open shader file " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(&buffer[0], fileSize);

    file.close();

    return buffer;
}