#version 410

in vec4 position;
in vec2 texCoord;
in vec4 color;

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 transform;

void main(void) {
    gl_Position = transform * position;
    vTexCoord = texCoord;
    vColor = color;
}
