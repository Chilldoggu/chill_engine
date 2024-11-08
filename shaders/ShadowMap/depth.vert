#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 light_view;
uniform mat4 light_projection;
uniform mat4 model;

void main() {
    gl_Position = light_projection * light_view * model * vec4(aPos, 1.0);
}
