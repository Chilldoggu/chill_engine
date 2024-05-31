#version 330 core

struct Light {
	vec3 color;
};

in vec2 TexCord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform Light light_source;

void main() {
	FragColor = vec4(light_source.color, 1.0f);
}
