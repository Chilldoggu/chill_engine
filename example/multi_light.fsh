#version 330 core

#define POINTLIGHT_NUM 25

struct DirLight {
	// General
	vec3 dir;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;
};

struct PointLight {
	// General
	vec3 pos;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;

	// Point light
	float linear;
	float constant;
	float quadratic;
};

struct SpotLight {
	// General
	vec3 pos;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;

	// Point light
	float linear;
	float constant;
	float quadratic;

	// Spotlight
	float inner_cutoff;
	float outer_cutoff;
	vec3 spot_dir;
};

struct Material {
	sampler2D diffuse_map;
	sampler2D specular_map;
	sampler2D emission_map;
	float shininess;
};

in vec2 TexCord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform Material material;
uniform DirLight dirlight_source;
uniform PointLight pointlight_sources[POINTLIGHT_NUM];
uniform SpotLight spotlight_source;
uniform vec3 view_pos;

vec3 calc_dirlight(DirLight a_light, vec3 normal, vec3 view_dir);
vec3 calc_pointlight(PointLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 calc_spotlight(SpotLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main() {
	vec3 view_dir = normalize(view_pos - FragPos); // TO observer
	vec3 normal = normalize(Normal);

	vec3 frag_light = calc_dirlight(dirlight_source, normal, view_dir);
	for (int i = 0; i < POINTLIGHT_NUM; i++) {
		if (pointlight_sources[i].color != vec3(0.0)) {
			frag_light += calc_pointlight(pointlight_sources[i], normal, FragPos, view_dir);
		}
	}
	frag_light += calc_spotlight(spotlight_source, normal, FragPos, view_dir);

	FragColor = vec4(frag_light, 1.0);
	// FragColor = vec4(texture(material.specular_map, TexCord).rgb, 1.0);
	// FragColor = vec4(texture(material.emission_map, TexCord).rgb, 1.0);
}


vec3 calc_dirlight(DirLight a_light, vec3 normal, vec3 view_dir) {
	// Base
	vec3 light_dir = normalize(-a_light.dir);

	// Ambient, Diffuse, Specular
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 ambient  = a_light.ambient_intens  * texture(material.diffuse_map, TexCord).rgb;
	vec3 diffuse  = a_light.diffuse_intens  * diff * texture(material.diffuse_map, TexCord).rgb;
	vec3 specular = a_light.specular_intens * spec * texture(material.specular_map, TexCord).rgb;

	return a_light.color * (ambient + diffuse + specular);
}

vec3 calc_pointlight(PointLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	// Base
	vec3 light_dir = normalize(a_light.pos - frag_pos);

	// Ambient, Diffuse, Specular
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 ambient  = a_light.ambient_intens  * texture(material.diffuse_map, TexCord).rgb;
	vec3 diffuse  = a_light.diffuse_intens  * diff * texture(material.diffuse_map, TexCord).rgb;
	vec3 specular = a_light.specular_intens * spec * texture(material.specular_map, TexCord).rgb;

	// Attenuation
	float distance = length(a_light.pos - FragPos);
	float attenuation = 1 / (a_light.constant + a_light.linear * distance + a_light.quadratic * distance * distance);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return a_light.color * (ambient + diffuse + specular);
}

vec3 calc_spotlight(SpotLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	// Base
	vec3 light_dir = -normalize(frag_pos - a_light.pos);

	// Ambient, Diffuse, Specular
	vec3 ambient  = a_light.ambient_intens  * texture(material.diffuse_map, TexCord).rgb;
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 diffuse  = a_light.diffuse_intens  * diff * texture(material.diffuse_map, TexCord).rgb;
	vec3 specular = a_light.specular_intens * spec * texture(material.specular_map, TexCord).rgb;

	// Attenuation
	float distance = length(a_light.pos - FragPos);
	float attenuation = 1 / (a_light.constant + a_light.linear * distance + a_light.quadratic * distance * distance);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	// Soft edges
	float theta = dot(light_dir, normalize(-a_light.spot_dir));
	float epsilon = a_light.inner_cutoff - a_light.outer_cutoff;
	float intensity = clamp((theta - a_light.outer_cutoff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;
	
	return a_light.color * (ambient + diffuse + specular);
}
