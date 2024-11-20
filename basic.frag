#version 330 core

in vec2 chTex;  // Texture coordinates from the vertex shader
out vec4 outCol;

uniform sampler2D uTex;  // Texture sampler

void main() {
    outCol = texture(uTex, chTex);  // Sample the texture at the given coordinates
}

