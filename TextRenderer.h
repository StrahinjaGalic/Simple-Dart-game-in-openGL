#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <string>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

class TextRenderer {
public:
    TextRenderer(const std::string& fontPath, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, int fontSize);
    ~TextRenderer();

    void RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

private:
    unsigned int compileShader(GLenum type, const std::string& source);
    unsigned int createShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    void loadCharacters(const std::string& fontPath, int fontSize);

    unsigned int shaderProgram;
    unsigned int VAO, VBO;
    std::map<char, Character> Characters;
};

#endif // TEXTRENDERER_H

