#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include <GL/glew.h>

class Crosshair {
public:
    Crosshair();
    ~Crosshair();
    void initialize();
    void render();
    void update(float dt);
    void setPosition(float nx, float ny);
    float getX() const;
    float getY() const;

private:
    GLuint VAO, VBO, shaderProgram;
    float x, y, shakeAmount;

    unsigned int compileShader(GLenum type, const char* source);
};

#endif



