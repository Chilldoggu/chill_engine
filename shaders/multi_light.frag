#version 420 core

#define POINTLIGHT_NUM 25
#define MAX_SAMPLER_SIZ 4

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
uniform PointLight pointlight_sources[POINTLIGHT_NUM];
uniform SpotLight spotlight_source;
uniform float near_plane;
uniform float far_plane;
uniform float fog_dens;
uniform vec3 fog_color;
uniform vec3 view_pos;
uniform bool is_blinn_phong;

// Light calculations
vec3 calc_dirlight(DirLight a_light, vec3 normal, vec3 view_dir);
vec3 calc_pointlight(PointLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 calc_spotlight(SpotLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir);

// Fog calculations
float calc_lindepth(float a_depth_val, float a_near, float a_far);
vec3 calc_fog(float a_density, float a_lindepth, vec3 a_frag_base_color, vec3 a_fog_color);

void main() {
	vec3 view_dir = normalize(view_pos - FragPos); // TO observer
	vec3 normal = Normal; 

	float alpha = texture(material.diffuse_maps[0], TexCoord).a;

	if (alpha <= 0.1) discard;

	vec3 frag_light = calc_dirlight(dirlight_source, normal, view_dir);
	for (int i = 0; i < POINTLIGHT_NUM; i++) {
		if (pointlight_sources[i].color != vec3(0.0)) {
			frag_light += calc_pointlight(pointlight_sources[i], normal, FragPos, view_dir);
		}
	}
	frag_light += calc_spotlight(spotlight_source, normal, FragPos, view_dir);
	vec3 fog_val = calc_fog(fog_dens, calc_lindepth(gl_FragCoord.z, near_plane, far_plane), frag_light, fog_color);

	FragColor = vec4(fog_val, alpha);
}

float phong_spec(vec3 view_dir, vec3 light_dir, vec3 normal, float shininess) {
	vec3 reflected_dir = reflect(-light_dir, normal);
	return pow(max(dot(view_dir, reflected_dir), 0.0), shininess); 
}

float blinn_phong_spec(vec3 view_dir, vec3 light_dir, vec3 normal, float shininess) {
	vec3 halfway_dir = normalize(light_dir + view_dir);
	return pow(max(dot(normal, halfway_dir), 0.0), shininess * 2); 
}

vec3 calc_ambient(vec3 ambient_intens, sampler2D ambient_map) {
	return ambient_intens  * texture(ambient_map, TexCoord).rgb; 
}

vec3 calc_diffuse(vec3 diff_intens, sampler2D diff_map, vec3 normal, vec3 light_dir) {
	float diff = max(dot(normal, light_dir), 0.0);
	return diff_intens * diff * texture(diff_map, TexCoord).rgb; 
}

vec3 calc_specular(vec3 spec_intens, sampler2D spec_map, vec3 normal, vec3 light_dir, vec3 view_dir) { 
	float spec = 0.f;
	if (is_blinn_phong == true) {
		spec = blinn_phong_spec(view_dir, light_dir, normal, material.shininess);
	} else {
		spec = phong_spec(view_dir, light_dir, normal, material.shininess);
	} 
	return spec_intens * spec * texture(material.specular_maps[0], TexCoord).rgb; 
}

vec3 calc_dirlight(DirLight a_light, vec3 normal, vec3 view_dir) {
	// Base
	vec3 light_dir = normalize(-a_light.dir); 

	vec3 ambient  = calc_ambient(a_light.ambient_intens, material.diffuse_maps[0]);
	vec3 diffuse  = calc_diffuse(a_light.diffuse_intens, material.diffuse_maps[0], normal, light_dir);
	vec3 specular = calc_specular(a_light.specular_intens, material.specular_maps[0], normal, light_dir, view_dir); 

	return a_light.color * (ambient + diffuse + specular);
}

vec3 calc_pointlight(PointLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	// Base
	vec3 light_dir = normalize(a_light.pos - frag_pos);

	// Ambient, Diffuse, Specular
	vec3 ambient  = calc_ambient(a_light.ambient_intens, material.diffuse_maps[0]);
	vec3 diffuse  = calc_diffuse(a_light.diffuse_intens, material.diffuse_maps[0], normal, light_dir);
	vec3 specular = calc_specular(a_light.specular_intens, material.specular_maps[0], normal, light_dir, view_dir); 

	// Attenuation
	float dist = length(a_light.pos - FragPos);
	float denominator = a_light.constant + a_light.linear * dist + a_light.quadratic * dist * dist;
	float attenuation = (denominator == 0) ? 0 : (1 / denominator);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return a_light.color * (ambient + diffuse + specular);
}

vec3 calc_spotlight(SpotLight a_light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	// Base
	vec3 light_dir = -normalize(frag_pos - a_light.pos);

	// Ambient, Diffuse, Specular
	vec3 ambient  = calc_ambient(a_light.ambient_intens, material.diffuse_maps[0]);
	vec3 diffuse  = calc_diffuse(a_light.diffuse_intens, material.diffuse_maps[0], normal, light_dir);
	vec3 specular = calc_specular(a_light.specular_intens, material.specular_maps[0], normal, light_dir, view_dir); 

	// Attenuation
	float dist = length(a_light.pos - FragPos);
	float denominator = a_light.constant + a_light.linear * dist + a_light.quadratic * dist * dist;
	float attenuation = (denominator == 0) ? 0 : (1 / denominator);

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

// Return lindepth in range <near_plane / far_plane, 1>
float calc_lindepth(float a_depth_val, float a_near, float a_far) {
	float ndc = a_depth_val * 2 - 1;
	float lindepth = (2.0 * a_near * a_far) / (a_far + a_near - ndc * (a_far - a_near));
	return lindepth / far_plane;
}

vec3 calc_fog(float a_density, float a_lindepth, vec3 a_frag_base_color, vec3 a_fog_color) {
	vec3 depth_vec = vec3(exp(-pow(a_lindepth * a_density, 2.0)));
	return mix(a_fog_color, a_frag_base_color, depth_vec);
}
