#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform float threshold;

void main(){
	vec3 color = texture(scene, TexCoords).rgb;
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

	float contribution = max(brightness = threshold, 0.0) / max(brightness,1e-4);
	FragColor = vec4(color * contribution, 1.0);
}