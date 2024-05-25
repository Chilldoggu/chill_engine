#version 330 core

struct Light {
	vec3 pos;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCord;
layout (location = 3) in vec3 aNormal;

out vec2 TexCord;
out vec3 Normal;
out vec3 FragPos;
out Light LightSource;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_mat;
uniform Light light_source;

void main() {
	TexCord = aTexCord;
	Normal = normal_mat * aNormal;
	FragPos = vec3(view * model * vec4(aPos, 1.0f));
	LightSource.pos = vec3(view * vec4(light_source.pos, 1.0f));
	LightSource.color = light_source.color;
	LightSource.ambient_intens = light_source.ambient_intens;
	LightSource.diffuse_intens = light_source.diffuse_intens;
	LightSource.specular_intens = light_source.specular_intens;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
