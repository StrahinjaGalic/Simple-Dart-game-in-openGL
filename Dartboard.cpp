#include "Dartboard.h"
#include <iostream>
#include <cmath> // For sin, cos
#include <fstream>
#include <sstream>
#include "stb_image.h"

// Constants
const float Dartboard::RADIUS = 0.5f;

Dartboard::Dartboard(const char* texturePath, const char* vertexShaderPath, const char* fragmentShaderPath) {
    generateCircleVertices(); // Generate circle vertices

    // Load the dartboard texture
    textureID = loadImageToTexture(texturePath);

    // Set up OpenGL buffers for rendering the dartboard
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    unsigned int stride = (2 + 2) * sizeof(float); // Position (2 floats) + texture coordinates (2 floats)

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Load and compile shaders
    shaderProgram = createShader(vertexShaderPath, fragmentShaderPath);
}

Dartboard::~Dartboard() {
    glDeleteTextures(1, &textureID);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

void Dartboard::generateCircleVertices() {
    vertices.push_back(0.0f); // Center X
    vertices.push_back(0.0f); // Center Y
    vertices.push_back(0.5f); // Texture S
    vertices.push_back(0.5f); // Texture T

    // Generate the vertices for the circle (triangle fan)
    for (int i = 0; i <= NUM_SEGMENTS; ++i) {
        float angle = 2.0f * 3.14 * i / NUM_SEGMENTS;
        float x = RADIUS * cos(angle);
        float y = RADIUS * sin(angle);
        float s = 0.5f + 0.5f * cos(angle);
        float t = 0.5f - 0.5f * sin(angle); 

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(s);
        vertices.push_back(t);
    }
}

void Dartboard::render() {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Render the dartboard
    glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_SEGMENTS + 2);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

unsigned int Dartboard::loadImageToTexture(const char* filePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);

    if (!data) {
        std::cout << "Failed to load texture!" << std::endl;
        return 0;
    }

    GLint internalFormat = (channels == 4) ? GL_RGBA : GL_RGB;
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return textureID;
}

unsigned int Dartboard::compileShader(GLenum type, const char* sourcePath) {
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
    }
    return shader;
}

unsigned int Dartboard::createShader(const char* vertexShaderPath, const char* fragmentShaderPath) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderPath);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
