#version 330

uniform mat4 mvp;
uniform float magnetSize;

layout(points) in;
layout(triangle_strip,max_vertices=4) out;

in vec3  color[];
out vec2 texCoord;
out vec3 magColor;

void main() {
    float size = magnetSize;

    gl_Position = mvp * (vec4(gl_in[0].gl_Position.xy,0,1) + vec4(-size,-size,0,0));
    texCoord = vec2(-1,-1);
    magColor = color[0];
    EmitVertex();

    gl_Position = mvp * (vec4(gl_in[0].gl_Position.xy,0,1) + vec4( size,-size,0,0));
    texCoord = vec2( 1,-1);
    magColor = color[0];
    EmitVertex();

    gl_Position = mvp * (vec4(gl_in[0].gl_Position.xy,0,1) + vec4(-size, size,0,0));
    texCoord = vec2(-1, 1);
    magColor = color[0];
    EmitVertex();

    gl_Position = mvp * (vec4(gl_in[0].gl_Position.xy,0,1) + vec4( size, size,0,0));
    texCoord = vec2( 1, 1);
    magColor = color[0];
    EmitVertex();

    EndPrimitive();
}

