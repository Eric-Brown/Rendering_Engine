#ifndef VULK_UNIFORMBUFFEROBJECT
#define VULK_UNIFORMBUFFEROBJECT

#include "ExternalHeaders.h"

struct UniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

#endif