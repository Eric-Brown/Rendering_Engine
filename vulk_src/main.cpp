//
// Created by alexa on 5/18/2019.
//


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

#include "Triangleapplication.h"
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

int main(int argc, char **argv) {
	using namespace std;
	triangleapplication app;
	try {
		app.run();
	}
	catch (exception &exception1) {
		cerr << exception1.what() << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
