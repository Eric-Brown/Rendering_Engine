//
// Created by alexa on 3/23/2020.
//

#ifndef DNDIDEA_MODEL_H
#define DNDIDEA_MODEL_H

#include "ExternalHeaders.h"
#include "VulkanMemoryManager.h"
#include "VulkanImageManager.h"
#include "Vertex.h"

class Model
{
private:
	std::string textureFileName{};
	std::vector<Vertex> mesh{};
	std::vector<uint32_t> meshIndexes{};
	glm::mat4 model_transform{1.0f};
	std::tuple<vk::Buffer, VmaAllocation> vertexBuffer{};
	std::tuple<vk::Buffer, VmaAllocation> indexBuffer{};
	std::tuple<vk::Image, VmaAllocation> textureBuffer{};
	vk::ImageView textureImageView{};
	vk::Sampler textureSampler{};
	uint32_t mipLevels{};

public:
	Model(const std::string fName);

	Model(const std::string fName, const std::string texFName);

	~Model() noexcept;

	const std::tuple<vk::Buffer, VmaAllocation> &GetMeshBuffer();

	const std::tuple<vk::Buffer, VmaAllocation> &GetIndicesBuffer();

	const std::tuple<vk::Image, VmaAllocation> &GetTextureBuffer();

	const glm::mat4 GetModelMatrix();

	void ScaleModel(glm::vec3 delta);
	void TranslateModel(glm::vec3 delta);
	void RotateModel(glm::vec3 axis, float theta);
	const vk::Sampler GetTextureSampler();
	const vk::ImageView GetTextureView();

	uint32_t GetIndexCount();

	const glm::mat4 &GetModelTransform();

	void loadDataToGPU();

private:
	bool readModelFile(const std::string &pFile);

	void processSceneObject(const aiScene *scene);

	void createTextureImage();
	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags,
								  uint32_t inMipLevels);
	void createTextureImageView();
	void createTextureSampler();
	void loadImageDataToGPU();
	void DestroyBufferIfExists(std::tuple<vk::Buffer, VmaAllocation> buffer) noexcept;
};

#endif // DNDIDEA_MODEL_H
