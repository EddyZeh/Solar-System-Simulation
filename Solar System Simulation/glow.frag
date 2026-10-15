#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D glowTex;
uniform vec3 glowColor;

void main()
{
    vec4 tex = texture(glowTex, TexCoords);
    FragColor = vec4(glowColor, 1.0) * tex;
}
