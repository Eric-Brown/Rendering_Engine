//
// Created by alexa on 3/23/2020.
//

#include "Model.h"

const std::tuple<vk::Buffer, VmaAllocation> &Model::GetMeshBuffer() { return vertexBuffer; }

const std::tuple<vk::Buffer, VmaAllocation> &Model::GetIndicesBuffer() { return indexBuffer; }

const std::tuple<vk::Image, VmaAllocation> &Model::GetTextureBuffer()
{
	return textureBuffer;
}

const vk::Sampler Model::GetTextureSampler() { return textureSampler; }
const vk::ImageView Model::GetTextureView() { return textureImageView; }

uint32_t Model::GetIndexCount()
{
	return static_cast<uint32_t>(meshIndexes.size());
}

const glm::mat4 &Model::GetModelTransform() { return model_transform; }

bool Model::readModelFile(const std::string &pFile)
{
	Assimp::Importer importer;
	importer.SetPropertyBool("GLOB_MEASURE_TIME", true);
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE,
								  aiDefaultLogStream_STDOUT);
	std::cout << "Reading file now...\n"
			  << std::endl;
	const aiScene *scene = importer.ReadFile(
		pFile, aiProcess_ValidateDataStructure |
				   aiProcess_RemoveRedundantMaterials |
				   aiProcess_FindDegenerates | aiProcess_Triangulate |
				   aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{
		std::cerr << "Could not read file" << std::endl;
		return false;
	}
	Assimp::DefaultLogger::kill();
	std::cout << "Processing Geometry now...\n"
			  << std::endl;
	processSceneObject(scene);
	return true;
}

void Model::processSceneObject(const aiScene *scene)
{
	using namespace std;
	cout << "There are: " << scene->mNumMeshes << " meshes stored." << endl;
	auto root = scene->mRootNode;
	cout << "Root has " << root->mNumChildren << " children." << endl;
	cout << "Root also has: " << root->mNumMeshes << " meshes associated with it."
		 << endl;
	//set correctly here for now
	aiMesh *pAiMesh{scene->mMeshes[0]};
	for (size_t i{0}; i < root->mNumChildren; i++)
	{
		if (root->mChildren[i]->mNumMeshes)
		{
			//load meshes...
		}
	}
	cout << "There are: " << scene->mNumMaterials << " materials" << endl;
	if (pAiMesh->HasTextureCoords(0))
	{
		cout << "The given mesh has texture coordinates" << endl;
		cout << "There are " << pAiMesh->GetNumUVChannels() << " UV channels"
			 << endl;
	}
	if (pAiMesh->HasVertexColors(0))
	{
		cout << "The given mesh has vertex colors" << endl;
	}
	cout << "Going to read in: " << pAiMesh->mNumVertices
		 << " amount of vertices." << endl;
	mesh.resize(pAiMesh->mNumVertices);
	meshIndexes.clear();
	int channel = 0;
	cout << "Using channel " << channel << " for getting UV coord" << endl;
	for (size_t i{}; i < pAiMesh->mNumVertices; i++)
	{
		auto vertex_point = pAiMesh->mVertices[i];
		auto color_point = pAiMesh->mTextureCoords[channel][i];
		Vertex toAdd{{vertex_point[0], vertex_point[1], vertex_point[2]},
					 {1.0f, 1.0f, 1.0f},
					 {color_point[0], color_point[1]}};

		mesh[i] = toAdd;
	}
	cout << "There are now " << mesh.size() << " vertices read in" << endl;
	cout << "There are " << pAiMesh->mNumFaces << " number of faces." << endl;
	for (size_t i{}; i < pAiMesh->mNumFaces; i++)
	{
		for (size_t j{}; j < pAiMesh->mFaces[i].mNumIndices; j++)
		{
			meshIndexes.push_back(pAiMesh->mFaces[i].mIndices[j]);
		}
	}
	cout << "There are now: " << meshIndexes.size() << " number of indices"
		 << endl;
}

const glm::mat4 Model::GetModelMatrix()
{
	return model_transform;
}

void Model::ScaleModel(glm::vec3 delta)
{
	glm::scale(model_transform, delta);
}
void Model::TranslateModel(glm::vec3 delta)
{
	glm::translate(model_transform, delta);
}
void Model::RotateModel(glm::vec3 axis, float theta)
{
	glm::rotate(model_transform, theta, axis);
}
void Model::createTextureImage()
{
	auto handleInfo = VulkanImageManager::getInstance()->CreateImageFromFile(textureFileName);
	std::get<0>(textureBuffer) = handleInfo.image;
	std::get<1>(textureBuffer) = handleInfo.allocation;
	mipLevels = handleInfo.imageMipLevels;
	VulkanImageManager::getInstance()->GenerateMipmaps(handleInfo);
}

void Model::createTextureImageView()
{
	vk::ImageSubresourceRange subresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1};
	vk::ImageViewCreateInfo viewInfo{{}, std::get<0>(textureBuffer), vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, {}, subresourceRange};
	textureImageView = VulkanImageManager::getInstance()->CreateImageView(viewInfo);
}

void Model::createTextureSampler()
{
	vk::SamplerCreateInfo samplerInfo{{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 16.0f, VK_FALSE, {}, 0.0f, static_cast<float>(mipLevels), vk::BorderColor::eIntOpaqueBlack, VK_FALSE};
	textureSampler = VulkanImageManager::getInstance()->CreateImageSampler(samplerInfo);
}

void Model::loadImageDataToGPU()
{
	if (!textureFileName.empty())
	{
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
	}
}

Model::Model(const std::string fName)
{
	// consider checks
	readModelFile(fName);
}

Model::Model(const std::string fName, const std::string texFName) : Model(fName)
{
	textureFileName = {texFName};
}

Model::~Model() noexcept
{
	if (std::get<0>(textureBuffer) != vk::Image{})
	{
		VulkanImageManager::getInstance()->DestroyImage(std::get<0>(textureBuffer), std::get<1>(textureBuffer));
		VulkanImageManager::getInstance()->DestroyImageView(textureImageView);
		VulkanImageManager::getInstance()->DestroySampler(textureSampler);
	}
	DestroyBufferIfExists(vertexBuffer);
	DestroyBufferIfExists(indexBuffer);
}

void Model::DestroyBufferIfExists(std::tuple<vk::Buffer, VmaAllocation> buffer) noexcept
{
	if (std::get<0>(buffer) != vk::Buffer{})
	{
		VulkanMemoryManager::getInstance()->DestroyBuffer(std::get<0>(buffer), std::get<1>(buffer));
	}
}

void Model::loadDataToGPU()
{
	vertexBuffer = VulkanMemoryManager::getInstance()->createBufferTypeFromVector(
		mesh, vk::BufferUsageFlagBits::eVertexBuffer);
	indexBuffer = VulkanMemoryManager::getInstance()->createBufferTypeFromVector(
		meshIndexes, vk::BufferUsageFlagBits::eIndexBuffer);
	loadImageDataToGPU();
}
