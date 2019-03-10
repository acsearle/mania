#version 410

in vec2 vTexCoord;
out vec4 color;

uniform sampler2D sampler;

void main(void) {
    float r = texture(sampler, vTexCoord).r;
    color = vec4(r, r, r, r);
}
