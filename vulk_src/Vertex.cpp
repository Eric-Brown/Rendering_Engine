//
// Created by alexa on 2/11/2020.
//
#include <vulkan/vulkan.h>
#include <iostream>
#include "Vertex.h"

vk::VertexInputBindingDescription Vertex::getBindingDescription() {
	vk::VertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	std::cout << "Given size of Vertex: " << sizeof(Vertex) << std::endl;
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;
	return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
	std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);
	std::cout << "Pos Offset here: " << attributeDescriptions[1].offset << std::endl;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(Vertex, color);
//		attributeDescriptions[1].offset = 12;
	std::cout << "Color Offset here: " << attributeDescriptions[1].offset << std::endl;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
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
