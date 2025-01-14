#include "Dart.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath> // For M_PI

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Dart::Dart() : VAO(0), VBO(0), shaderProgram(0), isFlying(false) {}

Dart::~Dart() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

void Dart::initialize() {
    generateVertices();

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shaderProgram = createShader("dart.vert", "dart.frag");
}

void Dart::generateVertices() {
    const float bodyRadius = 0.01f;
    const float bodyLength = 0.1f;
    const float tipLength = 0.02f;
    const int segments = 20;

    // Generate cylinder body vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = bodyRadius * cos(angle);
        float y = bodyRadius * sin(angle);

        // Bottom circle
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);

        // Top circle
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(bodyLength);
    }

    // Generate cone tip vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = bodyRadius * cos(angle);
        float y = bodyRadius * sin(angle);

        // Base of the cone
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(bodyLength);

        // Tip of the cone
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(bodyLength + tipLength);
    }
}

void Dart::update(float deltaTime) {
    if (isFlying) {
        position += velocity * deltaTime;
        // Check if the dart has reached the target (simple distance check)
        if (glm::length(velocity) * deltaTime >= glm::length(position - velocity * deltaTime)) {
            isFlying = false;
        }
    }
}

void Dart::render(const glm::mat4& projection, const glm::mat4& view) {
    if (!isFlying) return;

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to point towards the dartboard

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 3);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Dart::throwDart(const glm::vec3& startPosition, const glm::vec3& targetPosition) {
    position = startPosition;
    velocity = glm::normalize(targetPosition - startPosition) * 5.0f; // Adjust speed as needed
    isFlying = true;
}

unsigned int Dart::compileShader(GLenum type, const char* sourcePath) {
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

unsigned int Dart::createShader(const char* vertexShaderPath, const char* fragmentShaderPath) {
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

