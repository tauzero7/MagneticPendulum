#version 330

uniform int isActiveMagnet;

in vec2 texCoord;
in vec3 magColor;

layout(location = 0) out vec4 fragColor;

void main() {
    float r = length(texCoord);
    if (r>1) discard;

    fragColor = vec4(magColor*(1-r*r*r*r),1);
    if (isActiveMagnet==1) {
        fragColor = mix(fragColor,vec4(1,1,0,1),r*r*r*r);
    }
}
