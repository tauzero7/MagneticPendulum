#version 330


uniform sampler2D tex;

layout(location = 0) out vec4 fragColor;
in vec2 texCoords;

void main() {
    fragColor = vec4(texCoords,0,1);
    fragColor = texture( tex, texCoords );
}
