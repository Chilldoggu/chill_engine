#version 420 core

in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 view_pos;
uniform samplerCube skybox_cubemap;

void main() {
	vec3 view_dir = normalize(FragPos - view_pos); // FROM observer
	vec3 reflection = reflect(view_dir, normalize(Normal));

	vec3 reflected_col = texture(skybox_cubemap, reflection).rgb;

	FragColor = vec4(reflected_col, 1.0);
} 
