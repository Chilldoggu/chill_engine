#version 330 core

struct Light {
	vec3 pos;
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
in Light LightSource;
out vec4 FragColor;

uniform Material material;
uniform float time;

void main() {
	// Ambient
	vec3 ambient_light = LightSource.ambient_intens * texture(material.diffuse_map, TexCord).rgb * LightSource.color;

	// Diffuse
	vec3 norm = normalize(Normal);
	vec3 light_dir = normalize(LightSource.pos - FragPos);
	float diff_scalar = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = LightSource.diffuse_intens * diff_scalar * texture(material.diffuse_map, TexCord).rgb * LightSource.color;

	// Specular
	vec3 view_dir = normalize(-FragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec_scalar = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 specular = LightSource.specular_intens * spec_scalar * texture(material.specular_map, TexCord).rgb * LightSource.color;

	vec3 emission = vec3(0.0);
	if (texture(material.specular_map, TexCord).rgb == vec3(0.0)) {
		emission = texture(material.emission_map, TexCord + vec2(0.0, time)).rgb;
	}

	vec3 perceived_color = (ambient_light + diffuse + specular + emission);

	FragColor = vec4(perceived_color, 1.0f);
}

