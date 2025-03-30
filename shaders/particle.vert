// shaders/particle.vert
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    vec4 pos = model * vec4(aPos, 0.0, 1.0);
    gl_Position = projection * view * pos;
}
