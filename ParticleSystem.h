// ParticleSystem.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
};

class ParticleSystem {
public:
    ParticleSystem();
    void initialize(const glm::vec3& origin, float impactAngle);
    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& projection);
    bool isFinished() const;
    void reset();
private:
    std::vector<Particle> particles;
    Shader* particleShader;
    unsigned int VAO, VBO;
    bool initialized;
    void initRenderData();
};
