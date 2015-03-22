#version 330

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 modelMX;

out vec3 vtexCoords;
out vec3 vnormal;

void main() {
    gl_Position = modelMX * in_position;
    vtexCoords = in_position.xyz;
    vnormal = in_normal;
}
