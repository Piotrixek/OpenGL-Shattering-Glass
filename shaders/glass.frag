// shaders/glass.frag
#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;
uniform vec3 lightPos = vec3(10.0, 10.0, 10.0);
uniform vec3 viewPos;
void main() {
    // Basic Phong shading with transparency for glass
    vec3 ambient = 0.2 * vec3(0.8, 0.9, 1.0);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8, 0.9, 1.0);
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 0.5);
}
