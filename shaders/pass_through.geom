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

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = projection * gl_in[i].gl_Position;
		Normal = vs_in[i].gs_Normal;
		FragPos = vs_in[i].gs_FragPos;
		TexCoord = vs_in[i].gs_TexCoord;
		EmitVertex(); 
	} 
	EndPrimitive();
}
