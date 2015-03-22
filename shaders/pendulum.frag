#version 330

layout(location = 0) out vec4 fragColor;

uniform float tScale;

in vec3 color;
in float time;

void main() {
    fragColor = vec4(vec3(1),1);
    fragColor = vec4(color*clamp(1.0 - log(tScale*time),0.2,1),1);
}
