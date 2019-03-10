#version 410

in vec4 position;
in vec2 texCoord;
out vec2 vTexCoord;

uniform mat4 transform;

void main(void) {
    gl_Position = transform * position;
    vTexCoord = texCoord;
}
