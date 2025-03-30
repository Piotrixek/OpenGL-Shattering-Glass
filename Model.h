// Model.h
#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

class Model {
public:
    Model(const std::string& path);
    void Draw(Shader& shader);
private:
    std::vector<Mesh> meshes;
    std::string directory;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};
