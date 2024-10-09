#version 330 core

out vec4 FragColor;

uniform float far_plane;
uniform float near_plane;

void main() {
	float ndc = gl_FragCoord.z * 2 - 1;
	float lindepth = (2.0 * near_plane * far_plane)/(far_plane + near_plane - ndc * (far_plane - near_plane));
	FragColor = vec4(vec3(lindepth / far_plane), 1.0);
}
