#include "Dartboard.h"
#include <iostream>
#include <cmath> // For sin, cos
#include <fstream>
#include <sstream>
#include "stb_image.h"

// Constants
const float Dartboard::RADIUS = 0.5f;
#define M_PI 3.14159265358979323846

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

    // Initialize marker resources
    setupMarker();
}

Dartboard::~Dartboard() {
    glDeleteTextures(1, &textureID);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &markerVBO);
    glDeleteVertexArrays(1, &markerVAO);
    glDeleteProgram(markerShaderProgram);
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

    // Render the hit markers after rendering the dartboard
    renderHitMarkers();
}

void Dartboard::setupMarker() {
    // Simple shader for hit markers
    const char* markerVertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        uniform vec2 offset;
        void main() {
            gl_Position = vec4(position + offset, 0.0, 1.0);
        }
    )";

    const char* markerFragmentShader = R"(
        #version 330 core
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color for markers
        }
    )";

    // Compile and link marker shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &markerVertexShader, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &markerFragmentShader, nullptr);
    glCompileShader(fragmentShader);

    markerShaderProgram = glCreateProgram();
    glAttachShader(markerShaderProgram, vertexShader);
    glAttachShader(markerShaderProgram, fragmentShader);
    glLinkProgram(markerShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up the VAO/VBO for the markers (simple square)
    float markerVertices[] = {
        -0.01f, -0.01f, // Bottom-left
         0.01f, -0.01f, // Bottom-right
         0.01f,  0.01f, // Top-right
        -0.01f,  0.01f  // Top-left
    };

    glGenVertexArrays(1, &markerVAO);
    glGenBuffers(1, &markerVBO);

    glBindVertexArray(markerVAO);

    glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(markerVertices), markerVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Dartboard::renderHitMarkers() {
    glUseProgram(markerShaderProgram);

    // Check if the shader program is valid
    if (markerShaderProgram == 0) {
        std::cerr << "Error: Marker shader program is not valid!" << std::endl;
        return;
    }

    glBindVertexArray(markerVAO);

    // Render each marker
    for (const auto& hit : hitPositions) {
        // Check if the uniform location is valid
        GLint offsetLocation = glGetUniformLocation(markerShaderProgram, "offset");
        if (offsetLocation == -1) {
            std::cerr << "Error: Uniform 'offset' not found!" << std::endl;
            continue;
        }

        // Set the uniform and render the marker
        glUniform2f(offsetLocation, hit.first, hit.second);
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL Error before glDrawArrays: " << err << std::endl;
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Check for any errors
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL Error after glDrawArrays: " << err << std::endl;
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}


void Dartboard::recordHit(float x, float y) {
    hitPositions.push_back(std::make_pair(x, y));
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

int Dartboard::calculateScore(float x, float y) {
    // Convert Cartesian coordinates to polar coordinates
    float radius = sqrt(x * x + y * y);
    float angle = atan2(y, x);  // Angle in radians

    // Normalize angle to [0, 2?)
    if (angle < 0) {
        angle += 2 * M_PI;  // Normalize negative angles
    }

    float rotationOffset = 80 * (M_PI / 180.0f);  // 5 degrees converted to radians
    angle -= rotationOffset;  // Apply the small rotation to the angle
    if (angle < 0) {
        angle += 2 * M_PI;
    }

    // Convert angle to sector (assuming 20 equally spaced sectors)
    int sector = (int)(angle / (2 * M_PI) * 20) % 20;

    // Debugging output for verification
    std::cout << "Hit Position: (" << x << ", " << y << "), "
        << "Radius: " << radius << ", "
        << "Angle (rad): " << angle << ", "
        << "Sector: " << sector << std::endl;

    // Score zones (adjust thresholds based on dartboard layout)
    if (radius <= 0.02f) return 50;  // Bullseye (inner red circle)
    if (radius <= 0.04f) return 25;  // Outer bullseye (green circle)
    if (radius > 0.38f) return 0;    // Outside the dartboard

    // Check double and triple rings
    if (radius > 0.35f && radius <= 0.38f) return sectors[sector] * 2;  // Double ring
    if (radius > 0.22f && radius <= 0.248f) return sectors[sector] * 3;  // Triple ring

    // Regular scoring
    return sectors[sector];
}
