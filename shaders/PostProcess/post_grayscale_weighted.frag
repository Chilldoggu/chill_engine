 #version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D fb_texture;

void main() {
	vec3 color_normal = texture(fb_texture, TexCoords).rgb;
	float grayscaled = 0.2126 * color_normal.r + 0.7152 * color_normal.g + 0.0722 * color_normal.b;
	FragColor = vec4(pow(vec3(grayscaled), vec3(1/2.2)), 1.0);
}
