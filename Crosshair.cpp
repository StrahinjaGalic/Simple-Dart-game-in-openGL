#include "Crosshair.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>  // For random shaking
#include <iostream>

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Crosshair::Crosshair() : x(0.0f), y(0.0f), shakeAmount(0.5f), shaderProgram(0) {}


Crosshair::~Crosshair() {
    // Cleanup OpenGL resources (e.g., deleting buffers)
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

// Compile a shader from source
unsigned int Crosshair::compileShader(GLenum type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
    }
    return shader;
}

void Crosshair::initialize() {
    // Vertex shader source
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        uniform mat4 model;
        void main() {
            gl_Position = model * vec4(aPos, 0.0, 1.0);
        }
    )";

    // Fragment shader source
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 crosshairColor; // Color as a uniform variable
        void main() {
            FragColor = vec4(crosshairColor, 1.0); // Use the provided color
        }
    )";


    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program for crosshair: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Crosshair geometry
    GLfloat vertices[] = {
        -0.05f, 0.0f,  // Left
         0.05f, 0.0f,  // Right
         0.0f, -0.05f, // Bottom
         0.0f, 0.05f   // Top
    };

    // Generate and bind VAO and VBO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Crosshair::update(float dt) {
    static float shakeTime = 0.0f;
    shakeTime += dt;

    // Increase the shake width with a broader random range
    float shakeFactor = 0.2f + 0.05f * sin(shakeTime * 2.0f);  // Vary shake width over time

    // Generate random shake values
    float randomX = ((rand() % 11) - 5) * shakeFactor;
    float randomY = ((rand() % 11) - 5) * shakeFactor;

    // Apply the shake with smoothing based on the time difference (dt)
    float smoothingFactor = 0.1f;  // Slightly higher smoothing factor for smoother movement
    x += (randomX - x) * smoothingFactor * dt;  // Use dt to make movement frame rate-independent
    y += (randomY - y) * smoothingFactor * dt;  // Use dt here as well

    // Clamp the values to prevent the crosshair from moving out of bounds
    x = clamp(x, -1.0f, 1.0f);
    y = clamp(y, -1.0f, 1.0f);
}









void Crosshair::render() {
    // Use the shader program
    glUseProgram(shaderProgram);

    // Set the crosshair position using a model matrix
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    if (modelLoc == -1) {
        std::cerr << "Failed to find uniform 'model' in shader!" << std::endl;
        return;  // If the uniform isn't found, exit rendering this frame
    }

    // Create translation matrix to move crosshair to (x, y)
    float model[16] = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);

    // Bind VAO and render the crosshair
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 4);  // Render 4 lines (crosshair)
    glBindVertexArray(0);

    // Unbind shader program
    glUseProgram(0);
}

void Crosshair::setColor(float r, float g, float b) {
    // Use the shader program
    glUseProgram(shaderProgram);

    // Find and set the crosshairColor uniform
    GLint colorLoc = glGetUniformLocation(shaderProgram, "crosshairColor");
    if (colorLoc != -1) {
        glUniform3f(colorLoc, r, g, b);
    }
    else {
        std::cerr << "Failed to find uniform 'crosshairColor' in shader!" << std::endl;
    }

    // Unbind the shader program
    glUseProgram(0);
}


void Crosshair::setPosition(float nx, float ny) {
    x = nx;
    y = ny;
}

float Crosshair::getX() const {
    return x;
}

float Crosshair::getY() const {
    return y;
}
