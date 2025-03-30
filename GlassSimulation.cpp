// GlassSimulation.cpp
#include "GlassSimulation.h"
#include "Globals.h"
#include "Logger.h"
#include <glm/gtc/matrix_transform.hpp>

constexpr float gravity = 9.81f;
constexpr float groundLevel = 0.0f;

GlassSimulation::GlassSimulation()
    : fallHeight(10.0f), impactAngle(45.0f), simulationTime(0.0f)
{
    state = State::FALLING;
    glassPosition = glm::vec3(0.0f, fallHeight, 0.0f);
    glassVelocity = glm::vec3(0.0f);
    // Load glass model from file
    glassModel = new Model("assets/glass.obj");
    if (!glassModel) {
        logger.addLog(" Failed to load glass model.");
    }
    glassShader = new Shader();
    unsigned int program = loadShader("shaders/glass.vert", "shaders/glass.frag");
    if (program == 0) {
        logger.addLog(" Failed to load glass shader.");
    }
    glassShader->ID = program;
}

GlassSimulation::~GlassSimulation() {
    delete glassModel;
    delete glassShader;
}

void GlassSimulation::resetSimulation() {
    simulationTime = 0.0f;
    state = State::FALLING;
    glassPosition = glm::vec3(0.0f, fallHeight, 0.0f);
    glassVelocity = glm::vec3(0.0f);
    particleSystem.reset();
}

void GlassSimulation::update(float dt) {
    simulationTime += dt;
    if (state == State::FALLING) {
        // Basic free-fall physics: s = 0.5 * g * t^2
        float displacement = 0.5f * gravity * simulationTime * simulationTime;
        glassPosition.y = fallHeight - displacement;
        if (glassPosition.y <= groundLevel) {
            glassPosition.y = groundLevel;
            state = State::SHATTERED;
            // Initialize particle system for shattered glass
            particleSystem.initialize(glassPosition, impactAngle);
            logger.addLog("Glass shattered.");
        }
    }
    else if (state == State::SHATTERED) {
        particleSystem.update(dt);
    }
}

void GlassSimulation::render(const glm::mat4& view, const glm::mat4& projection) {
    if (state == State::FALLING) {
        if (glassShader && glassModel) {
            glUseProgram(glassShader->ID);
            glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glassPosition);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "model"), 1, GL_FALSE, &modelMat[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "view"), 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
            glassModel->Draw(*glassShader);
        }
    }
    else if (state == State::SHATTERED) {
        particleSystem.render(view, projection);
    }
}
