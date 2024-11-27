#include "Button.h"
#include <iostream>
#include <fstream>
#include <sstream>

Button::Button(float x, float y, float width, float height)
    : x(x), y(y), width(width), height(height), shaderProgram(0), VAO(0), VBO(0), EBO(0) {
    // Initialize shader for a color button (no texture)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderProgram = createShader("button.vert", "button.frag");
    setupButtonGeometry(); // Initialize geometry for the button
}

// Destructor: Clean up OpenGL resources
Button::~Button() {
    std::cout << "Destroying Button..." << std::endl;
    if (VBO != 0) {
        std::cout << "Deleting VBO..." << std::endl;
        glDeleteBuffers(1, &VBO);
    }
    if (EBO != 0) {
        std::cout << "Deleting EBO..." << std::endl;
        glDeleteBuffers(1, &EBO);
    }
    if (VAO != 0) {
        std::cout << "Deleting VAO..." << std::endl;
        glDeleteVertexArrays(1, &VAO);
    }
    if (shaderProgram != 0) {
        std::cout << "Deleting shader program..." << std::endl;
        glDeleteProgram(shaderProgram);
    }
}

// Set the shader program for the button
void Button::setShader(unsigned int shaderProgram) {
    this->shaderProgram = shaderProgram;
}

// Set up the geometry for the button
void Button::setupButtonGeometry() {
    // Calculate half width and height for proper scaling and positioning
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    // Button vertices with position and color attributes
    float vertices[] = {
        // Positions                 // Colors
        x - halfWidth, y - halfHeight,   1.0f, 0.0f, 0.0f,  // Bottom-left
        x + halfWidth, y - halfHeight,   1.0f, 0.0f, 0.0f,  // Bottom-right
        x + halfWidth, y + halfHeight,   1.0f, 0.0f, 0.0f,  // Top-right
        x - halfWidth, y + halfHeight,   1.0f, 0.0f, 0.0f   // Top-left
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (index 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (index 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Helper to compile shader
unsigned int Button::compileShader(GLenum type, const char* sourcePath) {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open shader file: " << sourcePath << std::endl;
        return 0;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    const char* source = sourceCode.c_str();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// Create a shader program using vertex and fragment shaders
unsigned int Button::createShader(const char* vertexShaderPath, const char* fragmentShaderPath) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderPath);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Render the button
void Button::render() {
    // Use the shader program
    glUseProgram(shaderProgram);

    // Set the button color (using a uniform color for the button)
    GLint colorLoc = glGetUniformLocation(shaderProgram, "buttonColor");
    if (colorLoc != -1) {
        glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f); // Green color (can be changed)
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Render the button
    glBindVertexArray(0);
    std::cout << "Rendering Resume Button at position: " << x << ", " << y << std::endl;

}
