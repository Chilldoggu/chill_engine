 #version 420 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform CameraMatrices {
	uniform mat4 view;
	uniform mat4 projection; 
};

out vec3 TexCoords;

void main() {
	mat4 stripped_view = mat4(mat3(view));
	vec4 pos = projection * stripped_view * vec4(aPos, 1.0); 
    gl_Position = pos.xyww;
    TexCoords = aPos;
}
