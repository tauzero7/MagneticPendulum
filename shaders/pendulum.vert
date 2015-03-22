#version 330

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in float in_time;

uniform mat4 mvp;
uniform int  useSpherical;
uniform float pendulumLength;

out vec3 color;
out float time;

void main() {
    vec4 vert = vec4(in_position.xy,0,1);
    
    if (useSpherical==1) {
        float theta = in_position.x;
        float phi   = in_position.y;
       // vert.xy = pendulumLength*sin(theta)*vec2( cos(phi), sin(phi) );
    }
    
    gl_Position = mvp * vert;
    color = in_color.rgb;
    time = in_time;
}

