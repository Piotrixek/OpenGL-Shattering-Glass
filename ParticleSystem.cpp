// ParticleSystem.cpp
#include "ParticleSystem.h"
#include "Globals.h"
#include "Logger.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>

ParticleSystem::ParticleSystem() : initialized(false) {
    particleShader = new Shader();
    unsigned int program = loadShader("shaders/particle.vert", "shaders/particle.frag");
    if (program == 0) {
        logger.addLog("[ERROR] Failed to load particle shader.");
    }
    particleShader->ID = program;
    initRenderData();
}

void ParticleSystem::initRenderData() {
    // A simple quad for particle rendering
    float quadVertices[] = {
        -0.05f,  0.05f,
         0.05f, -0.05f,
        -0.05f, -0.05f,
        -0.05f,  0.05f,
         0.05f,  0.05f,
         0.05f, -0.05f
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    initialized = true;
}

void ParticleSystem::initialize(const glm::vec3& origin, float impactAngle) {
    particles.clear();
    int count = 100;
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = origin;
        // Random speed between 0 and 5
        float speed = ((rand() % 100) / 100.0f) * 5.0f;
        // Vary the angle slightly based on impactAngle
        float angle = impactAngle + ((rand() % 50) - 25) * 0.1f;
        float rad = glm::radians(angle);
        p.velocity = glm::vec3(speed * cos(rad), speed * sin(rad), ((rand() % 100) / 100.0f) * 5.0f);
        p.life = 3.0f;
        particles.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : particles) {
        if (p.life > 0.0f) {
            p.life -= dt;
            p.velocity.y -= 9.81f * dt;
            p.position += p.velocity * dt;
        }
    }
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(particleShader->ID);
    glUniformMatrix4fv(glGetUniformLocation(particleShader->ID, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(particleShader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glBindVertexArray(VAO);
    for (auto& p : particles) {
        if (p.life > 0.0f) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), p.position);
            glUniformMatrix4fv(glGetUniformLocation(particleShader->ID, "model"), 1, GL_FALSE, &model[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    glBindVertexArray(0);
}

bool ParticleSystem::isFinished() const {
    for (const auto& p : particles)
        if (p.life > 0.0f)
            return false;
    return true;
}

void ParticleSystem::reset() {
    particles.clear();
}
