#ifndef DARTBOARD_H
#define DARTBOARD_H

#include <vector>
#include <utility>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
struct DartHit {
    glm::vec3 position;
    glm::vec3 direction;
};

class Dartboard {
public:
    Dartboard(const char* texturePath, const char* vertexShaderPath, const char* fragmentShaderPath);
    ~Dartboard();
    void render(const glm::mat4& projection, const glm::mat4& view);
    int calculateScore(float x, float y, float zoomLevel);
    void recordHit(float x, float y);
    void renderHitMarkers();
    void clearHits();


private:
    std::vector<float> dartMeshVertices;
    std::vector<unsigned int> dartMeshIndices;
    unsigned int dartMeshVAO = 0, dartMeshVBO = 0, dartMeshEBO = 0;
    unsigned int dartVAO = 0, dartVBO = 0;
    unsigned int dartShaderProgram = 0;
    std::vector<DartHit> dartHits;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    unsigned int textureID;
    unsigned int markerVAO, markerVBO;
    unsigned int markerShaderProgram;
    static const int NUM_SEGMENTS = 100;
    static const float RADIUS;
    std::vector<float> vertices;
    int sectors[20] = { 20, 5, 12, 9, 14, 11, 8, 16, 7, 19, 3, 17, 2, 15, 10, 6, 13, 4, 18, 1 };
    std::vector<std::pair<float, float>> hitPositions;

    void generateCircleVertices();
    unsigned int loadImageToTexture(const char* filePath);
    unsigned int compileShader(GLenum type, const char* sourcePath);
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);
    void setupMarker();
    void setupDart();
    void renderDarts(const glm::mat4& projection, const glm::mat4& view);
    void renderSingleDart(const glm::vec3& pos, const glm::vec3& dir, const glm::mat4& projection, const glm::mat4& view);
    void setupDartMesh();
    void renderDartMesh(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
};

#endif // DARTBOARD_H
