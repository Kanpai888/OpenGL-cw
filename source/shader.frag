#version 330 core

precision highp float;

out vec3 gl_FragColor;

uniform vec3 fragColor;

void main(void) {
    gl_FragColor = fragColor;
}