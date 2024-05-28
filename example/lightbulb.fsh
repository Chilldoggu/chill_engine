#version 330 core

struct Light {
	vec3 pos;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;
};

in vec2 TexCord;
in vec3 Normal;
in vec3 FragPos;
in Light LightSource;
out vec4 FragColor;

void main() {
	FragColor = vec4(LightSource.color, 1.0f);
}
