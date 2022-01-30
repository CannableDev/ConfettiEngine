#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <fstream>

namespace Shader {
	std::vector<char> read(const std::string& filename);

};

#endif