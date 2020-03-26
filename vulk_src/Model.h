//
// Created by alexa on 3/23/2020.
//

#ifndef DNDIDEA_MODEL_H
#define DNDIDEA_MODEL_H

#include "ExternalHeaders.h"
#include "Vertex.h"

class Model {
private:
  std::vector<Vertex> mesh{};
  std::vector<uint32_t> meshIndexes{};
  glm::mat4 model_transform{1.0f};

public:
  //	Model(const char* fName);
  //	Model(const std::string fName);
  Model(std::vector<Vertex> &preloadedMesh,
        std::vector<uint32_t> preloadedMeshIndices);

  Model(std::string fName);

  const std::vector<Vertex> &GetMesh();

  const std::vector<uint32_t> &GetIndices();

  const glm::mat4 &GetModelTransform();

  void loadDataToGPU();

private:
  bool readModelFile(const std::string &pFile);

  void processSceneObject(const aiScene *scene);
};

#endif // DNDIDEA_MODEL_H
