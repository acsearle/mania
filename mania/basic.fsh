#version 410

in vec2 vTexCoord;
out vec4 color;

uniform sampler2D sampler;

void main(void) {
    color = texture(sampler, vTexCoord);
}
