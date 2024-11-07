 #version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 aModelMat;

uniform mat4 light_view;
uniform mat4 light_projection;

void main() { 
	gl_Position = light_projection * light_view * aModelMat * vec4(aPos, 1.0);
}
