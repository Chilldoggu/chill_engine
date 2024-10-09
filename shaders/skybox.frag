 #version 420

in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox_cubemap;

void main() {
	FragColor = texture(skybox_cubemap, TexCoords);
}
