#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Background {
public:
    Background(const std::string& texturePath); // Constructor with the texture path
    ~Background();
    void render(const glm::mat4& projection, const glm::mat4& view); // Render the background with projection and view matrices

private:
    GLuint VAO, VBO, textureID, shaderProgram; // OpenGL objects
    void loadTexture(const std::string& texturePath); // Load texture from file
};

