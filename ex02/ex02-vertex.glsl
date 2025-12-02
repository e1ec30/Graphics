#version 300 es
void main() {
    gl_Position = vec4(sin(float(gl_VertexID)),
                       cos(float(gl_VertexID)),
                       0,
                       1);
}

