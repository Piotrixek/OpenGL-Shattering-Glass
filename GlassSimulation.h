// GlassSimulation.h
#pragma once
#include "Model.h"
#include "ParticleSystem.h"
#include "Shader.h"
#include <glm/glm.hpp>

class GlassSimulation {
public:
    GlassSimulation();
    ~GlassSimulation();
    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void resetSimulation();
    // User-adjustable parameters via ImGui
    float fallHeight;   // starting height of the glass
    float impactAngle;  // influences the dispersion of fragments
private:
    enum class State { FALLING, SHATTERED };
    State state;
    float simulationTime;
    glm::vec3 glassPosition;
    glm::vec3 glassVelocity;
    Model* glassModel; // Loaded via Assimp (see Model.cpp)
    Shader* glassShader;
    ParticleSystem particleSystem;
};
