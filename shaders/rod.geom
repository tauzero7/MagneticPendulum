#version 330

uniform mat4 projMX;
uniform mat4 viewMX;

uniform float pendulumHeight;
uniform float pendulumLength;
uniform vec2  pos;

layout(points) in;
layout(line_strip,max_vertices=2) out;


void main() {
    vec3 v1 = vec3(0,0,pendulumHeight);

    float psi = asin(length(pos)/pendulumLength);
    vec3 v2 = vec3(pos,pendulumHeight - pendulumLength*cos(psi));

    gl_Position = projMX * viewMX * vec4(v1,1);
    EmitVertex();

    gl_Position = projMX * viewMX * vec4(v2,1);
    EmitVertex();

    EndPrimitive();
}

