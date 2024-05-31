#version 330 core

struct Light {
	vec3 pos_dir;
	vec3 color;
	vec3 ambient_intens;
	vec3 diffuse_intens;
	vec3 specular_intens;
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
uniform Light light_source;
uniform float time;
uniform vec3 view_light_pos;

void main() {
	// Ambient
	vec3 ambient_light = light_source.ambient_intens * texture(material.diffuse_map, TexCord).rgb * light_source.color;

	// Diffuse
	vec3 norm = normalize(Normal);
	vec3 light_dir = normalize(view_light_pos - FragPos);
	float diff_scalar = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = light_source.diffuse_intens * diff_scalar * texture(material.diffuse_map, TexCord).rgb * light_source.color;

	// Specular
	vec3 view_dir = normalize(-FragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec_scalar = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 specular = light_source.specular_intens * spec_scalar * texture(material.specular_map, TexCord).rgb * light_source.color;

	vec3 emission = vec3(0.0);
	if (texture(material.specular_map, TexCord).rgb == vec3(0.0)) {
		emission = texture(material.emission_map, TexCord + vec2(0.0, time)).rgb;
	}

	vec3 perceived_color = (ambient_light + diffuse + specular + emission);

	FragColor = vec4(perceived_color, 1.0f);
}

