#ifndef CALLBACKS_H
#define CALLBACKS_H
#include <GLFW/glfw3.h>
#include "camera.hpp"

bool firstMouse = true;

typedef struct {
    bool currentlyPressed;
    bool pressedLastFrame;
} KeyState;

typedef struct {
    KeyState keys[GLFW_KEY_LAST + 1];
} InputManager;

void initInputManager(InputManager* inputManager) {
    memset(inputManager, 0, sizeof(InputManager));
}

void updateInputManager(GLFWwindow* window, InputManager* inputManager) {
    for (int key = 0; key <= GLFW_KEY_LAST; key++) {
        inputManager->keys[key].pressedLastFrame = inputManager->keys[key].currentlyPressed;
        inputManager->keys[key].currentlyPressed = glfwGetKey(window, key) == GLFW_PRESS;
    }
}

bool wasKeyPressed(InputManager inputManager, int key) {
    return inputManager.keys[key].currentlyPressed && !inputManager.keys[key].pressedLastFrame;
}

bool wasKeyReleased(InputManager* inputManager, int key) {
    return !inputManager->keys[key].currentlyPressed && inputManager->keys[key].pressedLastFrame;
}

bool isKeyDown(InputManager inputManager, int key) {
    return inputManager.keys[key].currentlyPressed;
}

void processInput(GLFWwindow *window, Camera *camera, RenderState* renderState, InputManager inputManager, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        ProcessKeyboard(camera,FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        ProcessKeyboard(camera,BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        ProcessKeyboard(camera,LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        ProcessKeyboard(camera,RIGHT, deltaTime);
    }
    
    if (wasKeyPressed(inputManager, GLFW_KEY_L)) {renderState->sRGB = !renderState->sRGB;};
    if (wasKeyPressed(inputManager, GLFW_KEY_B)) {renderState->bloom = !renderState->bloom;};
    if (wasKeyPressed(inputManager, GLFW_KEY_SPACE)) {renderState->useNormal = !renderState->useNormal;};
    if (wasKeyPressed(inputManager, GLFW_KEY_P)) {renderState->useShadows = !renderState->useShadows;};
    if (wasKeyPressed(inputManager, GLFW_KEY_M)) {

        renderState->grabMouse = !renderState->grabMouse;
        if (renderState->grabMouse) {glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);}
        else {glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);}
    };
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (renderState->exposure > 0.0f)
            renderState->exposure -= 0.001f;
        else
            renderState->exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        renderState->exposure += 0.001f;
    }
    
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // float lastX = (float)WINDOW_WIDTH/2.0f;
    // float lastY = (float)WINDOW_HEIGHT/2.0f;
    static float lastX = 0;
    static float lastY = 0;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    ProcessMouseMovement(camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ProcessMouseScroll(camera, static_cast<float>(yoffset));
}

#endif
