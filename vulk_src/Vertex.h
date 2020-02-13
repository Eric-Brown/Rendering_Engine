//
// Created by alexa on 8/28/2019.
//

#ifndef DNDIDEA_VERTEX_H
#define DNDIDEA_VERTEX_H

#include <array>
#include <ostream>
#include "CommonIncludes.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

	friend std::ostream &operator<<(std::ostream &os, const Vertex &vertex);
};

#endif //DNDIDEA_VERTEX_H
