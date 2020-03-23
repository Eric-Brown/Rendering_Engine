//
// Created by alexa on 5/18/2019.
//

#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include "ExternalIncludes.h"
#include "TriangleApplication.h"
#include <iostream>
#include <stdexcept>



int main(int , char **) {
	using namespace std;
	TriangleApplication app;
	try {
		app.run();
	}
	catch (exception &exception1) {
		cerr << exception1.what() << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
