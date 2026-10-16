#version 330 core

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D diffuse0;
uniform vec3 viewPos;
uniform PointLight sunLight;
uniform float shininess;
uniform float emission;

void main(){
    vec3 texColor = vec3(texture(diffuse0, TexCoords));

    if( emission > 0.0){
        vec3 viewDirE = normalize(viewPos - FragPos);
        float mu = max(dot(normalize(Normal), viewDirE), 0.0);
        float limb = 1.0 - 0.6 * (1.0 - mu);
        FragColor = vec4(texColor * emission * limb, 1.0);
        return;
    }

    vec3 ambient = sunLight.ambient * texColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunLight.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = sunLight.diffuse * diff * texColor;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfWay = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfWay), 0.0), shininess);
    vec3 specular = sunLight.specular * spec;

    vec3 color = (ambient + diffuse + specular);
    FragColor = vec4(color, 1.0);
}