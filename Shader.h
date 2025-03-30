// Shader.h
#pragma once
#include <string>
#include <glad/glad.h>

std::string readFile(const char* path);
unsigned int compileShader(GLenum type, const std::string& source);
unsigned int loadShader(const char* vertexPath, const char* fragmentPath);

class Shader {
public:
    unsigned int ID;
    Shader() : ID(0) {}
    Shader(const char* vertexPath, const char* fragmentPath) {
        ID = loadShader(vertexPath, fragmentPath);
    }
    void use() const {
        glUseProgram(ID);
    }
   
};
