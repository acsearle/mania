#version 410

in vec4 position;
in vec2 texCoord;
out vec2 vTexCoord;

void main(void) {
    gl_Position = position;
    vTexCoord = texCoord;
}
