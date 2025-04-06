#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include "../thirdparty/glm/glm.hpp"
#include "../thirdparty/glm/gtc/matrix_transform.hpp"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

void UpdateCameraVectors(struct Camera& camera);

struct Camera {
    // Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructors
    Camera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch),
          Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        UpdateCameraVectors(*this);
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Position(glm::vec3(posX, posY, posZ)), WorldUp(glm::vec3(upX, upY, upZ)),
          Yaw(yaw), Pitch(pitch),
          Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        UpdateCameraVectors(*this);
    }
};

glm::mat4 GetViewMatrix(const Camera& camera) {
    return glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
}

void ProcessKeyboard(Camera& camera, Camera_Movement direction, float deltaTime) {
    float velocity = camera.MovementSpeed * deltaTime;
    if (direction == FORWARD)
        camera.Position += camera.Front * velocity;
    if (direction == BACKWARD)
        camera.Position -= camera.Front * velocity;
    if (direction == LEFT)
        camera.Position -= camera.Right * velocity;
    if (direction == RIGHT)
        camera.Position += camera.Right * velocity;
}

void ProcessMouseMovement(Camera& camera, float xoffset, float yoffset, bool constrainPitch = true) {
    xoffset *= camera.MouseSensitivity;
    yoffset *= camera.MouseSensitivity;

    camera.Yaw   += xoffset;
    camera.Pitch += yoffset;

    if (constrainPitch) {
        if (camera.Pitch > 89.0f)
            camera.Pitch = 89.0f;
        if (camera.Pitch < -89.0f)
            camera.Pitch = -89.0f;
    }

    UpdateCameraVectors(camera);
}

void ProcessMouseScroll(Camera& camera, float yoffset) {
    camera.Zoom -= yoffset;
    if (camera.Zoom < 1.0f)
        camera.Zoom = 1.0f;
    if (camera.Zoom > 45.0f)
        camera.Zoom = 45.0f;
}

void UpdateCameraVectors(Camera& camera) {
    glm::vec3 front;
    front.x = cos(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
    front.y = sin(glm::radians(camera.Pitch));
    front.z = sin(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
    camera.Front = glm::normalize(front);

    camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
    camera.Up    = glm::normalize(glm::cross(camera.Right, camera.Front));
}
#endif