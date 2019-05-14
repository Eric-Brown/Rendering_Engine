//
// Created by alexa on 5/13/2019.
//

#include <fstream>
#include <string>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>

#ifndef DNDIDEA_SHADER_HELPER_H
#define DNDIDEA_SHADER_HELPER_H

class Shader_Helper {
public:
	static unsigned int FromFile(const std::string &path, gl::GLenum type) {
		using namespace std;
		ifstream file{path};
		if (!file) {
			throw runtime_error("Could not open provided path.");
		}
		string src{};
		while (file) {
			string temp{};
			getline(file, temp);
			src += temp + "\n";
		}
		auto shader_handle = gl::glCreateShader(type);
		const char *source = src.c_str();
		gl::glShaderSource(shader_handle, 1, &source, nullptr);
		gl::glCompileShader(shader_handle);
		return shader_handle;
	}
};

#endif //DNDIDEA_SHADER_HELPER_H
