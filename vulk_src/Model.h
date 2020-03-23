//
// Created by alexa on 3/23/2020.
//

#ifndef DNDIDEA_MODEL_H
#define DNDIDEA_MODEL_H

#include "ExternalIncludes.h"
#include "Vertex.h"

class Model {
private:
	std::vector<Vertex> mesh{};
	std::vector<uint32_t> indices{};
	glm::mat4 model_transform{1.0f};
public:
//	Model(const char* fName);
//	Model(const std::string fName);
	Model(std::vector<Vertex>& preloadedMesh, std::vector<uint32_t> preloadedMeshIndices);
	const std::vector<Vertex>& GetMesh();
	const std::vector<uint32_t>& GetIndices();
	const glm::mat4& GetModelTransform();
};


#endif //DNDIDEA_MODEL_H
