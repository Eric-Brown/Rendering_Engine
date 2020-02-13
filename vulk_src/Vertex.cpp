//
// Created by alexa on 2/11/2020.
//
#include <vulkan/vulkan.h>
#include <iostream>
#include "Vertex.h"

VkVertexInputBindingDescription Vertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	std::cout << "Given size of Vertex: " << sizeof(Vertex) << std::endl;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);
	std::cout << "Pos Offset here: " << attributeDescriptions[1].offset << std::endl;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);
//		attributeDescriptions[1].offset = 12;
	std::cout << "Color Offset here: " << attributeDescriptions[1].offset << std::endl;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
//		attributeDescriptions[2].offset = 24;
	std::cout << "UV Offset here: " << attributeDescriptions[2].offset << std::endl;
	return attributeDescriptions;
}

std::ostream &operator<<(std::ostream &os, const Vertex &vertex) {
	os << "Position: " << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << std::endl;
	os << "Color: " << vertex.color.r << ", " << vertex.color.g << ", " << vertex.color.b << std::endl;
	os << "TexCoord: " << vertex.texCoord.r << ", " << vertex.texCoord.g << std::endl;
	return os;
}
