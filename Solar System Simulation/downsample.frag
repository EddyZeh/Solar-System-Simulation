#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;   
void main()
{
    vec2 t = 1.0 / textureSize(image, 0);
    vec3 c = texture(image, TexCoords + t * vec2(-1.0, -1.0)).rgb
           + texture(image, TexCoords + t * vec2( 1.0, -1.0)).rgb
           + texture(image, TexCoords + t * vec2(-1.0,  1.0)).rgb
           + texture(image, TexCoords + t * vec2( 1.0,  1.0)).rgb;
    FragColor = vec4(c * 0.25, 1.0);
}
