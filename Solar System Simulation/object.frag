#version 330 core

in vec3 vertexColor;
in vec2 vUV;

uniform sampler2D uTexture;

out vec4 FragColor;

uniform float uTime;

void main(){
	
	vec3 texColor = texture(uTexture, vUV).rgb;

	float intensity = (sin(uTime) + 1.0) * 0.5f;
	//vec3 animatedColor = vertexColor * intensity;
	FragColor = vec4(texColor * intensity, 1.0);
}