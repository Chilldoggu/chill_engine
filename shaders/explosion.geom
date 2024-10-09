#version 420 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

in VS_OUT {
	vec3 gs_Normal;
	vec2 gs_TexCoord;
	vec3 gs_FragPos; 
} vs_in[];

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform float time;

const vec3 g_vnormals[] = vec3[](
	mat3(view) * vs_in[0].gs_Normal,
	mat3(view) * vs_in[1].gs_Normal,
	mat3(view) * vs_in[2].gs_Normal
);

vec4 explode2(vec4 pos) {
	float magnitude = 3.2;
	float freq = ((sin(time) + 1.0) / 2.0);
	vec4 direction = vec4(g_vnormals[0] + g_vnormals[1] + g_vnormals[2], 0) * magnitude * freq;
	return pos + direction;
} 

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = projection * explode2(gl_in[i].gl_Position);
		Normal = vs_in[i].gs_Normal;
		FragPos = vs_in[i].gs_FragPos;
		TexCoord = vs_in[i].gs_TexCoord;
		EmitVertex(); 
	} 
	EndPrimitive();
}
