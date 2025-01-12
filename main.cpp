#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Dartboard.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "Crosshair.h"
#include "Player.h"
#include "Background.h"
#include "TextRenderer.h"
#include "Overlay.h" // Assuming you have an Overlay class

void processInput(GLFWwindow* window);
void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard, TextRenderer& textRenderer, const glm::mat4& projection, const glm::mat4& view);
void processThrow(Player& currentPlayer, Dartboard& dartboard, const glm::mat4& projection, const glm::mat4& view);
void checkOpenGLError(const char* description);

// Game objects
Player player1("Player 1");
Player player2("Player 2");
Crosshair crosshair;

int currentPlayer = 0;  // 0 for player1, 1 for player2
float lastTime = 0.0f;
bool throwProcessed = false; // Prevent multiple frames registering a click
bool isPaused = false;       // Tracks whether the game is paused
unsigned int rectangleVAO;       // VAO for the rectangle
unsigned int rectangleShader;    // Shader program for the rectangle

// Rectangle position and size (in NDC space)
const float rectX = 360.0f; // X position in NDC space
const float rectY = 390.0f; // Y position in NDC space
const float rectWidth = 0.4f; // Width in NDC
const float rectHeight = 0.2f; // Height in NDC
float zoomLevel = 1.0f;
float zoomFactor = 1.0f; // Zoom factor for the dartboard

float targetZoomLevel = 1.0f;
float currentZoomLevel = 1.0f;
float zoomSpeed = 0.1f; // Adjust the speed of zooming


const float TARGET_FPS = 60.0f;
const float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;



// Vertex Shader Source
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0f, 1.0f);
}
)";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
}
)";

// Rectangle Setup
unsigned int createRectangleVAO() {
    float scaleX = 0.4f;  // Scale factor for the width of the rectangle
    float scaleY = 0.2f;  // Scale factor for the height of the rectangle

    float vertices[] = {
        // Positions (NDC)
        -0.5f * scaleX,  0.5f * scaleY, // Top-left
         0.5f * scaleX,  0.5f * scaleY, // Top-right
         0.5f * scaleX, -0.5f * scaleY, // Bottom-right
        -0.5f * scaleX,  0.5f * scaleY, // Top-left
         0.5f * scaleX, -0.5f * scaleY, // Bottom-right
        -0.5f * scaleX, -0.5f * scaleY  // Bottom-left
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}



unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // Check for compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link Shaders into Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}





void renderRectangle(TextRenderer& textRenderer) {
    glUseProgram(rectangleShader);
    glBindVertexArray(rectangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6); // Draw 6 vertices (2 triangles)
    glBindVertexArray(0);

    // Calculate position of the rectangle (centered on the screen)
    float rectWidth = 0.4f; // Width of the rectangle in NDC
    float rectHeight = 0.2f; // Height of the rectangle in NDC

    // Center the rectangle in the middle of the screen (NDC space)
    float rectX = 360.0f;
    float rectY = 390.0f;

    // Adjust text size and position to appear centered above the rectangle
    float textWidth = 150.0f;  // Width of the text
    float textHeight = 120.0f;  // Height of the text

    // Center the text horizontally on the screen and above the rectangle
    float textX = rectX - textWidth / 2.0f / 800.0f;  // Horizontal center of the screen, adjust for text width
    float textY = rectY + rectHeight / 2.0f + 0.05f; // Place text above the rectangle (with some gap)

    // Render the "Quit" text inside the rectangle
    textRenderer.RenderText("Quit", textX, textY, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
}

void checkQuitClick(GLFWwindow* window) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert mouse position to normalized device coordinates (NDC)
    float normX = (float)mouseX / (float)width * 2.0f - 1.0f;  // X coordinate in NDC
    float normY = 1.0f - (float)mouseY / (float)height * 2.0f; // Y coordinate in NDC

    // Print the normalized device coordinates when clicked
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        std::cout << "Mouse position (NDC): " << normX << ", " << normY << std::endl;

        // Check if mouse click is inside the quit button's NDC bounds
        if (normX >= -0.2f && normX <= 0.195f &&
            normY >= -0.095f && normY <= 0.1025f) {
            std::cout << "Quit button clicked!" << std::endl;
            glfwSetWindowShouldClose(window, true); // Close the window to end the game
        }
    }
}


int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Dartboard", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW!" << std::endl;
        return -1;
    }

    Background background("barBackground.jpg"); // Load the background texture
    Dartboard dartboard("Dartboard.png", "basic.vert", "basic.frag");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    TextRenderer textRenderer("Jaro-Regular.ttf", "text.vert", "text.frag", 48);
    TextRenderer nameRenderer("Jaro-Regular.ttf", "text.vert", "text.frag", 20);
    Overlay overlay("overlay.vert", "overlay.frag", "button.vert", "button.frag"); // Initialize the overlay
    TextRenderer quitRenderer("Jaro-Regular.ttf", "text.vert", "text.frag", 48);
    TextRenderer arrowRendere("Jaro-Regular.ttf", "text.vert", "text.frag", 48);

    crosshair.initialize();  // Initialize the crosshair
    crosshair.setColor(1.0f, 0.0f, 0.0f);  // Start with Player 1's color (Red)

    rectangleVAO = createRectangleVAO();
    rectangleShader = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Calculate the time to sleep in order to maintain 60 FPS
        float frameTime = 1.0f / TARGET_FPS;

        if (deltaTime < frameTime) {
            // Sleep for the remainder of the frame time if deltaTime is less than the target
            glfwWaitEventsTimeout(frameTime - deltaTime);
            currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
        }
        lastTime = currentTime;

        // Process input
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);

        // Create projection and view matrices
        float fov = 45.0f / currentZoomLevel; // Adjust FOV based on currentZoomLevel
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 800.0f, 0.1f, 100.0f);
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f / currentZoomLevel); // Move camera closer based on currentZoomLevel
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        if (isPaused) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            // Render game objects first
            background.render(projection, view);
            checkOpenGLError("Background rendering");

            dartboard.render(projection, view);
            checkOpenGLError("Dartboard rendering");

            crosshair.render();
            checkOpenGLError("Crosshair rendering");

            // Update the game if not paused
            updateGame(deltaTime, window, dartboard, textRenderer, projection, view);

            // Render player's name and details
            nameRenderer.RenderText("RA 156/2021", 0.0f, 780.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            nameRenderer.RenderText("Strahinja Galic", 0.0f, 760.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            // Now render the paused overlay
            overlay.renderPauseMenu();
            checkOpenGLError("Overlay rendering");

            renderRectangle(quitRenderer);
            checkOpenGLError("Rectangle rendering");

            checkQuitClick(window);

        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            background.render(projection,view);
            checkOpenGLError("Background rendering");

            dartboard.render(projection, view);
            checkOpenGLError("Dartboard rendering");

            crosshair.render();
            checkOpenGLError("Crosshair rendering");

            // Update the game if not paused
            updateGame(deltaTime, window, dartboard, textRenderer, projection, view);

            // Render player's name and details
            nameRenderer.RenderText("RA 156/2021", 0.0f, 780.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            nameRenderer.RenderText("Strahinja Galic", 0.0f, 760.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}






void processInput(GLFWwindow* window) {
    static bool escPressed = false;

    // Toggle pause menu with ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressed) {
        escPressed = true;
        isPaused = !isPaused;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        escPressed = false;
    }

    // Only process crosshair movement if the game is not paused
    if (!isPaused) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float normX = (mouseX / width) * 2.0f - 1.0f;  // X coordinate in NDC
        float normY = 1.0f - (mouseY / height) * 2.0f; // Y coordinate in NDC

        // Apply random shaking effect to the crosshair
        float shakeAmount = 0.02f;
        float shakeX = (rand() % 1000 - 500) / 500.0f * shakeAmount;
        float shakeY = (rand() % 1000 - 500) / 500.0f * shakeAmount;

        crosshair.setPosition(normX + shakeX, normY + shakeY);
    }

    // Handle zooming
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        targetZoomLevel -= 0.1f;
        if (targetZoomLevel < 0.1f) targetZoomLevel = 0.1f; // Prevent zooming too far in
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        targetZoomLevel += 0.1f;
        if (targetZoomLevel > 2.0f) targetZoomLevel = 2.0f; // Prevent zooming too far out
    }

    // Handle zooming with Alt key
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        currentZoomLevel = 1.1f; // Instantly zoom in
        std::cout << "Alt key pressed: Zooming in" << std::endl; // Debug output
    }
    else {
        currentZoomLevel = targetZoomLevel; // Reset zoom to target zoom level
    }
}







void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard, TextRenderer& textRenderer, const glm::mat4& projection, const glm::mat4& view) {
    static bool waitingToClear = false;
    static float clearStartTime = 0.0f;

    Player& current = (currentPlayer == 0) ? player1 : player2;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !throwProcessed && !isPaused) {
        if (current.getDartsLeft() > 0) {
            processThrow(current, dartboard, projection, view);
            throwProcessed = true;

            if (current.getScore() == 501) {
                std::cout << current.getName() << " wins with a perfect 501!" << std::endl;
                glfwSetWindowShouldClose(window, true); // Close the window to end the game
            }
            else if (current.getDartsLeft() == 0 && !waitingToClear) {
                waitingToClear = true;
                clearStartTime = glfwGetTime();
            }
        }
        else {
            std::cout << current.getName() << " has no darts left!" << std::endl;
        }
    }

    if (waitingToClear) {
        float elapsedTime = glfwGetTime() - clearStartTime;
        if (elapsedTime >= 1.0f) {
            dartboard.clearHits();
            currentPlayer = (currentPlayer == 0) ? 1 : 0;
            (currentPlayer == 0 ? player1 : player2).resetDarts();
            waitingToClear = false;

            crosshair.setColor((currentPlayer == 0) ? 1.0f : 0.0f, 0.0f, (currentPlayer == 1) ? 1.0f : 0.0f);
            std::cout << "Switching to " << (currentPlayer == 0 ? player1.getName() : player2.getName()) << std::endl;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        throwProcessed = false;
    }

    crosshair.update(deltaTime);
    std::string text = current.getName() + ": " + std::to_string(current.getScore());
    textRenderer.RenderText(text, 0.0f, 30.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    checkOpenGLError("Rendering player name text");

    // Display number of darts left
    std::string dartsLeftText = "Darts left: " + std::to_string(current.getDartsLeft());
    textRenderer.RenderText(dartsLeftText, -0.9f, 0.8f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
}


void processThrow(Player& currentPlayer, Dartboard& dartboard, const glm::mat4& projection, const glm::mat4& view) {
    float hitX = crosshair.getX();
    float hitY = crosshair.getY();

    // Adjust the scaling factor to match the visual dartboard space
    // Try different values between 0.5 and 1.0 to find the best match
    float scaleFactor = 0.95f; // Experiment with this value
    float dartboardX = hitX * scaleFactor;
    float dartboardY = hitY * scaleFactor;

    dartboard.recordHit(dartboardX, dartboardY);
    int points = dartboard.calculateScore(dartboardX, dartboardY);

    std::cout << currentPlayer.getName() << " hit (" << dartboardX << ", " << dartboardY
        << ") and scored " << points << " points!\n";

    currentPlayer.addScore(points);
    currentPlayer.throwDart();
}








void checkOpenGLError(const char* description) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (err) {
        case GL_INVALID_ENUM: error = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: error = "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: error = "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: error = "GL_OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        default: error = "Unknown error"; break;
        }
        std::cout << "OpenGL Error (" << description << "): " << error << std::endl;
    }


}
