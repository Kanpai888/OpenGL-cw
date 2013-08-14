#version 330 core

in vec3 in_position;
uniform mat4 MVP;

void main() {
    vec4 v = vec4(in_position, 1.0);
    gl_Position = MVP * v;
}
