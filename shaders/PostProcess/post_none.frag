 #version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D fb_texture;

void main() {
	vec4 ret = texture(fb_texture, TexCoords);
	ret.rgb = pow(ret.rgb, vec3(1/2.2));
	FragColor = ret;
}
