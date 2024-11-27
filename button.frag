#version 330 core
in vec3 fragColor;
out vec4 FragColor;

uniform vec3 buttonColor;

void main() {
    FragColor = vec4(buttonColor, 1.0f); // Use the button color
}

