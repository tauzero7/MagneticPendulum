#version 330

uniform vec3 lineColor;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(lineColor,1);
}
