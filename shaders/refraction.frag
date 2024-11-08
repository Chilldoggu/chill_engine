 #version 430 core

in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 view_pos;
uniform samplerCube skybox_cubemap;

void main() {
	float refract_ratio = 1.00 / 1.52;
	vec3 view_dir = normalize(FragPos - view_pos); // FROM observer
	vec3 refraction_vec = refract(view_dir, normalize(Normal), refract_ratio);

	vec3 refracted_col = texture(skybox_cubemap, refraction_vec).rgb;

	FragColor = vec4(refracted_col, 1.0);
} 
