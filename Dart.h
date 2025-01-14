#ifndef DART_H
#define DART_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Dart {
public:
    Dart();
    ~Dart();
    void initialize();
    void update(float deltaTime);
    void render(const glm::mat4& projection, const glm::mat4& view);
    void throwDart(const glm::vec3& startPosition, const glm::vec3& targetPosition);

private:
    void generateVertices();
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    std::vector<float> vertices;
    glm::vec3 position;
    glm::vec3 velocity;
    bool isFlying;
    unsigned int compileShader(GLenum type, const char* source);
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);
};

#endif // DART_H

