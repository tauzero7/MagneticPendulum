#version 330

uniform mat4 projMX;
uniform mat4 viewMX;
uniform int  haveNormals;

layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;

in  vec3 vtexCoords[];
in  vec3 vnormal[];

out vec3 texCoord;
out vec3 normal;
out vec3 pos;

void main() {
    vec4 v1 = gl_in[0].gl_Position;
    vec4 v2 = gl_in[1].gl_Position;
    vec4 v3 = gl_in[2].gl_Position;

    vec3 n = cross((v2-v1).xyz,(v3-v1).xyz);
    n = normalize(n);

    gl_Position = projMX * viewMX * v1;
    texCoord = vtexCoords[0];
    normal = haveNormals*vnormal[0] + (1-haveNormals)*n;
    pos = v1.xyz;
    EmitVertex();

    gl_Position = projMX * viewMX * v2;
    texCoord = vtexCoords[1];
    normal = haveNormals*vnormal[1] + (1-haveNormals)*n;
    pos = v2.xyz;
    EmitVertex();

    gl_Position = projMX * viewMX * v3;
    texCoord = vtexCoords[2];
    normal = haveNormals*vnormal[2] + (1-haveNormals)*n;
    pos = v3.xyz;
    EmitVertex();

    EndPrimitive();
}

