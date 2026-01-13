#version 300 es

layout(location=0) in vec4 position;
layout(location=1) in vec4 color;

uniform mat4 m;
out vec4 vColor;

void main() {

    vColor = color;
    gl_Position = position * m;
}

