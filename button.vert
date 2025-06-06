#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

out vec3 fragColor;

uniform mat4 model;

void main() {
    fragColor = color;
    gl_Position = model * vec4(position, 0.1f, 1.0f);
}





