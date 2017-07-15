#version 410

in vec4 position;
in vec4 color;
out vec4 vColor;

uniform vec4 mColor;

void main(void) {
    gl_Position = position;
    vColor = color * mColor;
}
