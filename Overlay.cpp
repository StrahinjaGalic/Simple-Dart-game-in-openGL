#include "Overlay.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Constructor: Initialize the shader program and overlay geometry
Overlay::Overlay(const char* vertexShaderPath, const char* fragmentShaderPath, const char* quitButtonVertexPath, const char* quitButtonFragmentPath) 
    : resumeButton(250.0f, 250.0f, 100.0f, 100.0f) {
    shaderProgram = createShader(vertexShaderPath, fragmentShaderPath); // General overlay shader
    setupOverlayGeometry();

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


}

// Destructor: Clean up resources
Overlay::~Overlay() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

// Set up VAO and VBO for a quad (used for button and other overlays)
void Overlay::setupOverlayGeometry() {
    // Geometry for the full screen overlay
    float vertices[] = {
        // Positions   // Texture Coords
        -1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
         1.0f, -1.0f,  1.0f, 0.0f, // Bottom-right
         1.0f,  1.0f,  1.0f, 1.0f, // Top-right
        -1.0f,  1.0f,  0.0f, 1.0f  // Top-left
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}


// Compile a shader from a file
unsigned int Overlay::compileShader(GLenum type, const char* sourcePath) {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open shader file: " << sourcePath << std::endl;
        return 0;  // Return an invalid shader
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
        glDeleteShader(shader);  // Clean up in case of failure
        return 0;  // Return an invalid shader
    }

    return shader;
}

// Create a shader program using vertex and fragment shaders
unsigned int Overlay::createShader(const char* vertexShaderPath, const char* fragmentShaderPath) {
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


// Render a pause menu with an overlay
void Overlay::renderPauseMenu() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);

    // Set the overlay color for the full screen (50% transparent black)
    GLint colorLoc = glGetUniformLocation(shaderProgram, "overlayColor");
    if (colorLoc != -1) {
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.8f); // Black with 50% opacity
    }
    else {
        std::cerr << "Error: Uniform 'overlayColor' not found in shader program!" << std::endl;
    }

    glBindVertexArray(VAO);
    // Render the semi-transparent overlay (covering the entire screen)
    renderTexturedQuad(0.0f, 0.0f, 2.0f, 2.0f); // Full screen

    glBindVertexArray(0);
    glUseProgram(0);

	//resumeButton.render();
}


// Helper to render a textured quad
void Overlay::renderTexturedQuad(float x, float y, float width, float height) {
    glBindVertexArray(VAO);

    // Modify vertices or use shader uniforms for position and size
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}