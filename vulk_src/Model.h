//
// Created by alexa on 3/23/2020.
//

#ifndef DNDIDEA_MODEL_H
#define DNDIDEA_MODEL_H

#include "ExternalHeaders.h"
#include "VulkanMemoryManager.h"
#include "Vertex.h"

class Model {
private:
  std::vector<Vertex> mesh{};
  std::vector<uint32_t> meshIndexes{};
  glm::mat4 model_transform{1.0f};
 std::tuple<vk::Buffer, VmaAllocation> vertexBuffer{};
 std::tuple<vk::Buffer, VmaAllocation> indexBuffer{};
public:

  Model(const std::string fName);

  ~Model() noexcept;

  const std::tuple<vk::Buffer, VmaAllocation> &GetMeshBuffer();

  const std::tuple<vk::Buffer, VmaAllocation> &GetIndicesBuffer();

  const uint32_t GetIndexCount();

  const glm::mat4 &GetModelTransform();

  void loadDataToGPU();

private:
  bool readModelFile(const std::string &pFile);

  void processSceneObject(const aiScene *scene);
};

#endif // DNDIDEA_MODEL_H
