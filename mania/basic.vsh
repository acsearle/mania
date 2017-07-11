#version 410

in vec4 position;
out vec4 vPosition;

void main(void) {
    gl_Position = position;
    vPosition = position;
}
