 #version 430 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
	vec3 gs_Normal;
} vs_in[];

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

uniform float magnitude;

void generate_line(int index) {
	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = projection * (gl_in[index].gl_Position + vec4(vs_in[index].gs_Normal, 0) * magnitude);
	EmitVertex();
	EndPrimitive();
}

void main() {
	for (int i = 0; i < 3; ++i) {
		generate_line(i);
	} 
}
