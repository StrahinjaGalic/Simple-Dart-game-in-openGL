#version 330 core
in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D dartboardTexture;

void main() {
    if (TexCoord == vec2(0.0f, 0.0f)) {
        FragColor = vec4(0.3f, 0.3f, 0.3f, 1.0f); // Lighter shade for sides
    } else {
        FragColor = texture(dartboardTexture, TexCoord);
    }
}








