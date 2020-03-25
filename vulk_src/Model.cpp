//
// Created by alexa on 3/23/2020.
//

#include "Model.h"

Model::Model(std::vector<Vertex> &preloadedMesh, std::vector<uint32_t> preloadedMeshIndices)
		: mesh(std::move(preloadedMesh)), indices(std::move(preloadedMeshIndices)) {
}

const std::vector<Vertex> &Model::GetMesh() {
	return mesh;
}

const std::vector<uint32_t> &Model::GetIndices() {
	return indices;
}

const glm::mat4 &Model::GetModelTransform() {
	return model_transform;
}
