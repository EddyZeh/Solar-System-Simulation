#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom0;
uniform sampler2D bloom1;
uniform sampler2D bloom2;
uniform sampler2D bloom3;
uniform sampler2D bloom4;
uniform float exposure;
uniform float bloomStrength;

void main(){
	vec3 hdr = texture(scene, TexCoords).rgb;

	vec3 bloomColor = texture(bloom0, TexCoords).rgb
					+ texture(bloom1, TexCoords).rgb
					+ texture(bloom2, TexCoords).rgb
					+ texture(bloom3, TexCoords).rgb
					+ texture(bloom4, TexCoords).rgb;

	vec3 color = hdr + bloomColor * (bloomStrength / 5.0);

	// tone mapping: compress HDR range into [0,1]
	color = vec3(1.0) - exp(-color * exposure);

	// gamma: computed in linear sight
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);

}