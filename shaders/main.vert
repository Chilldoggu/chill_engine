#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

out VS_OUT {
	vec3 gs_Normal;
	vec2 gs_TexCoord;
	vec3 gs_FragPos; 
} vs_out; 

uniform mat4 model;
uniform mat3 normal_mat;

void main() { 
	vs_out.gs_Normal = normalize(normal_mat * aNormal);
	vs_out.gs_TexCoord = aTexCoord;
	vs_out.gs_FragPos = vec3(model * vec4(aPos, 1.0));

	gl_Position = view * model * vec4(aPos, 1.0);
	gl_PointSize = 0.3;
}