 #version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D fb_texture;
uniform float gamma;

vec3 linear_to_sRGB(vec3 in_col) {
	vec3 ret;
	float a = 0.055;
	for (int comp = 0; comp < 3; ++comp) {
		if (in_col[comp] <= 0.0031308) {
			ret[comp] = 12.92 * in_col[comp];
		} else {
			ret[comp] = (1 + a) * pow(in_col[comp], 1.0/2.4) - a;
		}
	}
	return ret;
}

void main() {
	vec4 ret_color = texture(fb_texture, TexCoords);
	ret_color.rgb = linear_to_sRGB(ret_color.rgb);
	FragColor = ret_color;
}
