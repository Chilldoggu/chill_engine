 #version 430 core

in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 view_pos;
uniform samplerCube skybox_cubemap;

void main() {
	vec3 view_dir = normalize(FragPos - view_pos); // FROM observer
	vec3 reflection = reflect(view_dir, normalize(Normal)); 
	// vec3 refraction = refract(view_dir, normalize(Normal), 1.0/1.52); 

	FragColor = vec4(texture(skybox_cubemap, reflection).rgb, 1.0);
} 
