#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 color = vec3(1.0, 0.6, 0.2);  // orange sphere

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    // Ambient + diffuse
    vec3 result = (0.2 + diff) * color;
    FragColor = vec4(result, 1.0);
}

