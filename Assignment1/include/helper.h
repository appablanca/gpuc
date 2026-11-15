#pragma once
#include <string>

#if defined(WORKING_DIR)
inline std::string workingDir = std::string(WORKING_DIR) + "/";
#else
inline std::string workingDir = std::string("./");
#endif


inline std::string get_shader_path(std::string shader) {
	return workingDir + "build/shaders/" + shader + ".comp.spv";
}
