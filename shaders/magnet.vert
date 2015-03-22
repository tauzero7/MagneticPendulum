#version 330

uniform vec2 in_pos;
uniform vec3 in_col;

out vec3 color;

void main() {
    gl_Position = vec4(in_pos,0,1);
    color = in_col;
}

