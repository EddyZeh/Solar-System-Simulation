#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 sunPos;
uniform float size;

void main(){
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up    = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 worldPos = sunPos
                  + right * aPos.x * size
                  + up    * aPos.y * size;

    TexCoords = aTex;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
