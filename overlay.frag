#version 330 core

out vec4 FragColor;
uniform vec4 overlayColor;

void main() {
    FragColor = overlayColor; // Use the uniform color, including alpha for transparency
}
