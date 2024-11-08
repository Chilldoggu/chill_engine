 #version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

out VS_OUT {
	vec3 gs_Normal;
} vs_out;

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

uniform mat4 model;
uniform mat3 normal_mat;

void main() {
	vs_out.gs_Normal = normalize(vec3(view * vec4(normal_mat * aNormal, 0)));
	gl_Position = view * model * vec4(aPos, 1.0);
	gl_PointSize = 0.3;
}
