 #version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D fb_texture;

void main() {
	vec3 color_normal = texture(fb_texture, TexCoords).rgb;
	float grayscaled = (color_normal.r + color_normal.g + color_normal.b) / 3;
	FragColor = vec4(pow(vec3(grayscaled), vec3(1/2.2)), 1.0);
}
