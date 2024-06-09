#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCord;
layout (location = 3) in vec3 aNormal;

out vec2 TexCord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_mat;

void main() {
	TexCord = aTexCord;
	Normal = normal_mat * aNormal;
	FragPos = vec3(model * vec4(aPos, 1.0f));

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
