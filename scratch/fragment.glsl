#version 300 es
precision highp float;

in vec4 vColor;
out vec4 fragColor;
uniform mat4 m;

void main () {
	fragColor = vColor;
}
