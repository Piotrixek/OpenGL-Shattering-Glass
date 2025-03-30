// GlassSimulation.h
#pragma once
#include "Model.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>

struct FragmentSim {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotationAxis;
    float rotationAngle;
    float angularVelocity;
    Mesh* mesh;
};

class GlassSimulation {
public:
    GlassSimulation();
    ~GlassSimulation();
    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void resetSimulation();
    float fallHeight;   // Starting height of the glass
    float impactAngle;  // Controls fragment dispersion
private:
    enum class State { FALLING, SHATTERED, SIMULATION_DONE };
    State state;
    float simulationTime;
    glm::vec3 glassPosition;
    glm::vec3 glassVelocity;
    Model* glassModel;
    Shader* glassShader;
    std::vector<FragmentSim> fragments;
    unsigned int planeVAO, planeVBO;
    Shader* planeShader;
    void initPlane();
    void renderPlane(const glm::mat4& view, const glm::mat4& projection);
    const float gravity = 9.81f;
    const float restitution = 0.5f;
    const float friction = 0.8f;
};
