#version 330

layout(location = 0) in vec4 in_position;

uniform mat4 mvp;

void main() {
    vec4 vert = vec4(in_position.xy,0,1);
    gl_Position = mvp * vert;
}

