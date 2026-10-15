#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float exposure;

void main()
{
    vec3 hdr = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloom, TexCoords).rgb;

    vec3 color = hdr + bloomColor;

    // tone mapping
    color = vec3(1.0) - exp(-color * exposure);

    FragColor = vec4(color, 1.0);
}
