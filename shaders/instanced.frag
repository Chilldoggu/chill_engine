#version 420 core

#define MAX_SAMPLER_SIZ 4

struct DirLight {
	// General
	vec3 dir;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;
};

struct Material {
	sampler2D diffuse_maps[MAX_SAMPLER_SIZ];
	sampler2D specular_maps[MAX_SAMPLER_SIZ];
	sampler2D emission_maps[MAX_SAMPLER_SIZ];
	float shininess;
};

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform Material material;
uniform DirLight dirlight_source;
uniform vec3 view_pos;

vec3 calc_dirlight(DirLight a_light, vec3 normal, vec3 view_dir) {
	// Base
	vec3 light_dir = normalize(-a_light.dir);

	// Ambient, Diffuse, Specular
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 ambient  = a_light.ambient_intens  * texture(material.diffuse_maps[0], TexCoord).rgb;
	vec3 diffuse  = a_light.diffuse_intens  * diff * texture(material.diffuse_maps[0], TexCoord).rgb;
	vec3 specular = a_light.specular_intens * spec * texture(material.specular_maps[0], TexCoord).rgb;

	return a_light.color * (ambient + diffuse + specular);
}

void main() {
	vec3 view_dir = normalize(view_pos - FragPos); // TO observer
	vec3 normal = Normal; 

	float alpha = texture(material.diffuse_maps[0], TexCoord).a; 
	if (alpha <= 0.1) discard;

	vec3 frag_light = calc_dirlight(dirlight_source, normal, view_dir); 
	FragColor = vec4(frag_light, alpha);
}
