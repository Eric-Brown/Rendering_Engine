//
// Created by alexa on 3/23/2020.
//

#include "Model.h"

Model::Model(std::vector<Vertex> &preloadedMesh,
             std::vector<uint32_t> preloadedMeshIndices)
    : mesh(std::move(preloadedMesh)),
      meshIndexes(std::move(preloadedMeshIndices)) {}

const std::vector<Vertex> &Model::GetMesh() { return mesh; }

const std::vector<uint32_t> &Model::GetIndices() { return meshIndexes; }

const glm::mat4 &Model::GetModelTransform() { return model_transform; }

bool Model::readModelFile(const std::string &pFile) {
  // Create an instance of the Importer class
  Assimp::Importer importer;

  importer.SetPropertyBool("GLOB_MEASURE_TIME", true);
  Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE,
                                aiDefaultLogStream_STDOUT);
  std::cout << "Reading file now...\n" << std::endl;
  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll
  // probably to request more postprocessing than we do in this example.
  const aiScene *scene = importer.ReadFile(
      pFile,
      aiProcess_ValidateDataStructure | // Validates imported structure
          //	                                         aiProcess_MakeLeftHanded
          //| 	                                         	                                         aiProcess_FlipWindingOrder |
          //                                             aiProcess_PreTransformVertices
          //                                             |
          aiProcess_RemoveRedundantMaterials | // Removes duplicated materials
          //	                                         aiProcess_FindInstances
          //| 	                                         aiProcess_RemoveComponent |
          aiProcess_FindDegenerates |
          //	                                         aiProcess_GenUVCoords |
          aiProcess_Triangulate | aiProcess_FlipUVs |
          //                                             aiProcess_FindInvalidData
          //                                             |
          //                                             aiProcess_FixInfacingNormals
          //                                             |
          //                                             aiProcess_SplitLargeMeshes
          //                                             | aiProcess_SortByPType
          //                                             |
          //???
          //	                                         aiProcess_GenNormals |
          //	                                         aiProcess_CalcTangentSpace
          //| 	                                         aiProcess_OptimizeMeshes |
          //                                                                                          aiProcess_JoinIdenticalVertices |
          //	                                         aiProcess_LimitBoneWeights
          //| 	                                         aiProcess_ImproveCacheLocality |
          //                                             aiProcess_CalcTangentSpace
          //                                             |

          aiProcess_JoinIdenticalVertices |
          //	                                         aiProcess_SplitLargeMeshes
          //| 	                                                                                      aiProcess_SortByPType |
          //                                             aiProcess_FlipUVs |
          //                                             aiProcess_FlipWindingOrder
          //                                             |
          0);

  // If the import failed, report it
  if (!scene) {
    std::cerr << "Could not read file" << std::endl;
    //		DoTheErrorLogging( importer.GetErrorString());
    return false;
  }
  Assimp::DefaultLogger::kill();
  std::cout << "Processing Geometry now...\n" << std::endl;

  // Now we can access the file's contents.
  processSceneObject(scene);

  // We're done. Everything will be cleaned up by the importer destructor
  return true;
}

void Model::processSceneObject(const aiScene *scene) {
  using namespace std;
  cout << "There are: " << scene->mNumMeshes << " meshes stored." << endl;
  auto root = scene->mRootNode;
  cout << "Root has " << root->mNumChildren << " children." << endl;
  cout << "Root also has: " << root->mNumMeshes << " meshes associated with it."
       << endl;
  aiMesh *pAiMesh{scene->mMeshes[0]};
  if (root->mNumChildren) {
    cout << "Child of root:" << endl;
    auto child = root->mChildren[0];
    cout << "has " << root->mChildren[0]->mNumChildren << " amount of children"
         << endl;
    cout << "Child has: " << child->mNumMeshes << " meshes associated with it."
         << endl;
    pAiMesh = scene->mMeshes[child->mMeshes[0]];
  }
  cout << "There are: " << scene->mNumMaterials << " materials" << endl;
  if (pAiMesh->HasTextureCoords(0)) {
    cout << "The given mesh has texture coordinates" << endl;
    cout << "There are " << pAiMesh->GetNumUVChannels() << " UV channels"
         << endl;
  }
  if (pAiMesh->HasVertexColors(0)) {
    cout << "The given mesh has vertex colors" << endl;
  }
  cout << "Going to read in: " << pAiMesh->mNumVertices
       << " amount of vertices." << endl;
  mesh.resize(pAiMesh->mNumVertices);
  meshIndexes.clear();
  int channel = 0;
  cout << "Using channel " << channel << " for getting UV coord" << endl;
  for (size_t i{}; i < pAiMesh->mNumVertices; i++) {
    auto vertex_point = pAiMesh->mVertices[i];
    auto color_point = pAiMesh->mTextureCoords[channel][i];
    Vertex toAdd{{vertex_point[0], vertex_point[1], vertex_point[2]},
                 {1.0f, 1.0f, 1.0f},
                 {color_point[0], color_point[1]}};

    mesh[i] = toAdd;
  }
  cout << "There are now " << mesh.size() << " vertices read in" << endl;
  cout << "There are " << pAiMesh->mNumFaces << " number of faces." << endl;
  for (size_t i{}; i < pAiMesh->mNumFaces; i++) {
    for (size_t j{}; j < pAiMesh->mFaces[i].mNumIndices; j++) {
      meshIndexes.push_back(pAiMesh->mFaces[i].mIndices[j]);
    }
  }
  cout << "There are now: " << meshIndexes.size() << " number of indices"
       << endl;
}

Model::Model(std::string fName) {
  // consider checks
  readModelFile(fName);
}

void Model::loadDataToGPU() {}
