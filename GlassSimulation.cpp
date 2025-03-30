// GlassSimulation.cpp
#include "GlassSimulation.h"
#include "Globals.h"
#include "Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <array>
#include <random>

// Helper functions for vertex operations
static float computeArea(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    glm::vec3 a = v1.Position - v0.Position;
    glm::vec3 b = v2.Position - v0.Position;
    return 0.5f * glm::length(glm::cross(a, b));
}

static Vertex midpoint(const Vertex& v0, const Vertex& v1) {
    Vertex m;
    m.Position = (v0.Position + v1.Position) * 0.5f;
    m.Normal = glm::normalize(v0.Normal + v1.Normal);
    return m;
}

// Recursively subdivide a triangle into smaller triangles below a given area threshold
static std::vector<std::array<Vertex, 3>> subdivideTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, float threshold) {
    std::vector<std::array<Vertex, 3>> result;
    float area = computeArea(v0, v1, v2);
    if (area <= threshold) {
        // Add random perturbation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-0.005f, 0.005f);
        std::array<Vertex, 3> tri = { v0, v1, v2 };
        for (auto& v : tri) {
            v.Position.x += dis(gen);
            v.Position.y += dis(gen);
            v.Position.z += dis(gen);
        }
        result.push_back(tri);
    }
    else {
        Vertex m0 = midpoint(v0, v1);
        Vertex m1 = midpoint(v1, v2);
        Vertex m2 = midpoint(v2, v0);
        auto t1 = subdivideTriangle(v0, m0, m2, threshold);
        auto t2 = subdivideTriangle(m0, v1, m1, threshold);
        auto t3 = subdivideTriangle(m2, m1, v2, threshold);
        auto t4 = subdivideTriangle(m0, m1, m2, threshold);
        result.insert(result.end(), t1.begin(), t1.end());
        result.insert(result.end(), t2.begin(), t2.end());
        result.insert(result.end(), t3.begin(), t3.end());
        result.insert(result.end(), t4.begin(), t4.end());
    }
    return result;
}

// Create fragments from a single mesh using subdivision
static std::vector<Mesh> fragmentMesh(const Mesh& original, float areaThreshold) {
    std::vector<Mesh> fragments;
    // For each triangle in the mesh
    for (size_t i = 0; i < original.indices.size(); i += 3) {
        Vertex v0 = original.vertices[original.indices[i]];
        Vertex v1 = original.vertices[original.indices[i + 1]];
        Vertex v2 = original.vertices[original.indices[i + 2]];
        auto tris = subdivideTriangle(v0, v1, v2, areaThreshold);
        for (auto& tri : tris) {
            std::vector<Vertex> verts = { tri[0], tri[1], tri[2] };
            std::vector<unsigned int> inds = { 0, 1, 2 };
            fragments.push_back(Mesh(verts, inds));
        }
    }
    return fragments;
}

GlassSimulation::GlassSimulation()
    : fallHeight(10.0f), impactAngle(45.0f), simulationTime(0.0f)
{
    state = State::FALLING;
    glassPosition = glm::vec3(0.0f, fallHeight, 0.0f);
    glassVelocity = glm::vec3(0.0f);
    glassModel = new Model("assets/glass.obj");
    if (!glassModel) {
        logger.addLog(" Failed to load glass model.");
    }
    glassShader = new Shader("shaders/glass.vert", "shaders/glass.frag");
    if (!glassShader->ID) {
        logger.addLog(" Failed to load glass shader.");
    }
    initPlane();
}

GlassSimulation::~GlassSimulation() {
    delete glassModel;
    delete glassShader;
    delete planeShader;
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    // Free allocated fragment meshes
    for (auto& frag : fragments) {
        delete frag.mesh;
    }
}

void GlassSimulation::resetSimulation() {
    simulationTime = 0.0f;
    state = State::FALLING;
    glassPosition = glm::vec3(0.0f, fallHeight, 0.0f);
    glassVelocity = glm::vec3(0.0f);
    for (auto& frag : fragments) {
        delete frag.mesh;
    }
    fragments.clear();
}

void GlassSimulation::initPlane() {
    float planeVertices[] = {
         50.0f, 0.0f,  50.0f,
        -50.0f, 0.0f,  50.0f,
        -50.0f, 0.0f, -50.0f,
         50.0f, 0.0f, -50.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    planeShader = new Shader("shaders/plane.vert", "shaders/plane.frag");
    if (!planeShader->ID) {
        logger.addLog(" Failed to load plane shader.");
    }
}

void GlassSimulation::update(float dt) {
    simulationTime += dt;
    if (state == State::FALLING) {
        float displacement = 0.5f * gravity * simulationTime * simulationTime;
        glassPosition.y = fallHeight - displacement;
        if (glassPosition.y <= 0.0f) {
            glassPosition.y = 0.0f;
            state = State::SHATTERED;
            // Clear previous fragments and free memory
            for (auto& frag : fragments) {
                delete frag.mesh;
            }
            fragments.clear();
            // If more than one mesh exists, use them; otherwise, subdivide the single mesh
            if (glassModel->meshes.size() > 1) {
                for (auto& mesh : glassModel->meshes) {
                    std::vector<Mesh> fragMeshes = fragmentMesh(mesh, 0.005f);
                    for (auto& fragMesh : fragMeshes) {
                        FragmentSim frag;
                        frag.position = glassPosition;
                        float speed = ((rand() % 100) / 100.0f) * 5.0f;
                        float angleRad = glm::radians(impactAngle + ((rand() % 50) - 25) * 0.1f);
                        frag.velocity = glm::vec3(speed * cos(angleRad),
                            speed * sin(angleRad),
                            ((rand() % 100) / 100.0f) * 5.0f);
                        frag.rotationAxis = glm::normalize(glm::vec3(((rand() % 100) / 100.0f),
                            ((rand() % 100) / 100.0f),
                            ((rand() % 100) / 100.0f)));
                        frag.rotationAngle = 0.0f;
                        frag.angularVelocity = ((rand() % 100) / 100.0f) * 90.0f;
                        // Allocate a new mesh for this fragment
                        frag.mesh = new Mesh(fragMeshes.back());
                        fragMeshes.pop_back();
                        fragments.push_back(frag);
                    }
                }
            }
            else {
                std::vector<Mesh> fragMeshes = fragmentMesh(glassModel->meshes[0], 0.005f);
                for (auto& subMesh : fragMeshes) {
                    FragmentSim frag;
                    frag.position = glassPosition;
                    float speed = ((rand() % 100) / 100.0f) * 5.0f;
                    float angleRad = glm::radians(impactAngle + ((rand() % 50) - 25) * 0.1f);
                    frag.velocity = glm::vec3(speed * cos(angleRad),
                        speed * sin(angleRad),
                        ((rand() % 100) / 100.0f) * 5.0f);
                    frag.rotationAxis = glm::normalize(glm::vec3(((rand() % 100) / 100.0f),
                        ((rand() % 100) / 100.0f),
                        ((rand() % 100) / 100.0f)));
                    frag.rotationAngle = 0.0f;
                    frag.angularVelocity = ((rand() % 100) / 100.0f) * 90.0f;
                    frag.mesh = new Mesh(subMesh);
                    fragments.push_back(frag);
                }
            }
            logger.addLog("Glass shattered into fragments.");
        }
    }
    else if (state == State::SHATTERED) {
        bool allStopped = true;
        for (auto& frag : fragments) {
            if (glm::length(frag.velocity) > 0.05f)
                allStopped = false;
            frag.velocity.y -= gravity * dt;
            frag.position += frag.velocity * dt;
            frag.rotationAngle += frag.angularVelocity * dt;
            if (frag.position.y < 0.0f) {
                frag.position.y = 0.0f;
                frag.velocity.y = -frag.velocity.y * restitution;
                frag.velocity.x *= friction;
                frag.velocity.z *= friction;
                frag.angularVelocity *= friction;
            }
        }
        if (allStopped) {
            state = State::SIMULATION_DONE;
            logger.addLog("Fragment simulation complete.");
        }
    }
}

void GlassSimulation::renderPlane(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(planeShader->ID);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(planeShader->ID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(planeShader->ID, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(planeShader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void GlassSimulation::render(const glm::mat4& view, const glm::mat4& projection) {
    renderPlane(view, projection);
    glUseProgram(glassShader->ID);
    if (state == State::FALLING) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glassPosition);
        glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "model"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
        glassModel->Draw(*glassShader);
    }
    else if (state == State::SHATTERED || state == State::SIMULATION_DONE) {
        for (auto& frag : fragments) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), frag.position);
            model = glm::rotate(model, glm::radians(frag.rotationAngle), frag.rotationAxis);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "model"), 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "view"), 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(glassShader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
            frag.mesh->Draw(*glassShader);
        }
    }
}
