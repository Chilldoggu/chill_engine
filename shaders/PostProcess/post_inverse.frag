#version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D framebuffer_texture;
uniform vec3 inv_color;

void main() {
	vec3 color_normal = texture(framebuffer_texture, TexCoords).rgb;
	FragColor = vec4(pow(abs(inv_color - color_normal), vec3(1/2.2)), 1.0);
}