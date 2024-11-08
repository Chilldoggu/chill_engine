 #version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

uniform mat4 model;
uniform mat3 normal_mat;

void main() {
	FragPos = vec3(model * vec4(aPos, 1.0f)); 
	Normal = normal_mat * aNormal;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
