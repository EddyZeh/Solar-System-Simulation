#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform mat4 uTransform;

out vec3 vertexColor;
out vec2 vUV;

void main(){
	vUV = aUV;
	vertexColor = aColor;
	gl_Position = uTransform * vec4(aPos, 1.0);
}