#version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D framebuffer_texture;

uniform mat3 kernel;
uniform float offset;

vec2 sampled_pos[] = vec2[](
	vec2(-offset, offset),  // Top left
	vec2(0,		  offset),  // Top
	vec2(offset,  offset),  // Top right
	vec2(-offset, 0),       // Left
	vec2(0,       0),       // Center
	vec2(offset,  0),       // Right
	vec2(-offset, -offset), // Bottom left
	vec2(0,       -offset), // Bottom
	vec2(offset,  -offset)  // Bottom right
);

void main() { 
	vec3 ret = vec3(0);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			vec3 sampled_val = texture(framebuffer_texture, TexCoords + sampled_pos[i*3+j]).rgb;
			// Sample kernel from row major because of sampled_pos layout
			ret += sampled_val * kernel[j][i];
		}
	} 
	FragColor = vec4(pow(ret, vec3(1/2.2)), 1.0);
}
