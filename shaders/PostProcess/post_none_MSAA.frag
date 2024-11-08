 #version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2DMS fb_texture;
uniform int sample_size;

void main() {
	vec4 ret_color;
	for (int i = 0; i < sample_size; ++i) {
		ret_color +=  texelFetch(fb_texture, ivec2(gl_FragCoord.x, gl_FragCoord.y), i) / sample_size;
	}
	FragColor = vec4(pow(ret_color.rgb, vec3(1/2.2)), ret_color.a);
}
