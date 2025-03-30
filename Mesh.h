// Mesh.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void Draw(Shader& shader);
private:
    unsigned int VBO, EBO;
    void setupMesh();
};
