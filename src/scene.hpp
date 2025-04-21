#ifndef SCENE_H
#define SCENE_H
#include "shader.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "../thirdparty/glm/glm.hpp"

enum PassType
{
    SHADOW_PASS,
    DRAW_PASS
};

void setupLightsForShader(const Shader& shader, const Light& dirLight, const Light spotLight, const glm::vec3& lightColor, const glm::vec3* pointLightPositions, int pointLightCount) {
    useShader(shader);
    setLight("dirLight", dirLight, shader);
    setLight("spotLight", spotLight, shader);
    for (int i = 0; i < pointLightCount; i++) {
        Light point = {
            .type = LIGHT_TYPE_POINT,
            .position = pointLightPositions[i],
            .ambient = glm::vec3(0.3f),
            .diffuse = lightColor,
            .specular = glm::vec3(1.0f),
            .constant = 0.0f,
            .linear = 0.0f,
            .quadratic = 1.0f
        };
        std::string name = "pointLights[" + std::to_string(i) + "]";
        setLight(name.c_str(), point, shader);
    }
}

void renderScene(Shader shader, Mesh* cubeMesh, Mesh* grassMesh, Mesh* floorMesh, glm::vec3* cubePositions, Texture* shadow, Texture* cubeShadow,  unsigned int pass_type)
{
    useShader(shader);
    {        
        if (pass_type == SHADOW_PASS) {glCullFace(GL_FRONT);};
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            // model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            setMat4(shader, "model",  glm::value_ptr(model));
            drawMesh(cubeMesh, &shader, shadow, cubeShadow);
        }
        if (pass_type == SHADOW_PASS) {glCullFace(GL_BACK);};
        
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-3,-3.90 + 1.0,0));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            setMat4(shader, "model",  glm::value_ptr(model));
            drawMesh(grassMesh, &shader, shadow, cubeShadow);
        }
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0,-4,0));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            // model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            model = glm::scale(model, glm::vec3(30.0f, 30.0f, 0.1f));
            setMat4(shader, "model",  glm::value_ptr(model));
            drawMesh(floorMesh, &shader, shadow, cubeShadow);
        }
    }

};

// Meshs Data
Vertex cubeVertices[] = {
    // position           // normal            // tex coords

    // Front face
    {{-1.0f, -1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
    {{ 1.0f, -1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
    {{ 1.0f,  1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
    {{-1.0f,  1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},

    // Back face
    {{-1.0f, -1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {1.0f, 0.0f}},
    {{ 1.0f, -1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {0.0f, 0.0f}},
    {{ 1.0f,  1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {0.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {1.0f, 1.0f}},

    // Left face
    {{-1.0f, -1.0f, -1.0f},  {-1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
    {{-1.0f, -1.0f,  1.0f},  {-1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
    {{-1.0f,  1.0f,  1.0f},  {-1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f},  {-1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},

    // Right face
        {{1.0f, -1.0f, -1.0f},   {1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
        {{1.0f, -1.0f,  1.0f},   {1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
        {{1.0f,  1.0f,  1.0f},   {1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
        {{1.0f,  1.0f, -1.0f},   {1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},

    // Bottom face
    {{-1.0f, -1.0f, -1.0f},   {0.0f, -1.0f,  0.0f},   {0.0f, 1.0f}},
    {{ 1.0f, -1.0f, -1.0f},   {0.0f, -1.0f,  0.0f},   {1.0f, 1.0f}},
    {{ 1.0f, -1.0f,  1.0f},   {0.0f, -1.0f,  0.0f},   {1.0f, 0.0f}},
    {{-1.0f, -1.0f,  1.0f},   {0.0f, -1.0f,  0.0f},   {0.0f, 0.0f}},

    // Top face
    {{-1.0f,  1.0f, -1.0f},   {0.0f,  1.0f,  0.0f},   {0.0f, 0.0f}},
    {{ 1.0f,  1.0f, -1.0f},   {0.0f,  1.0f,  0.0f},   {1.0f, 0.0f}},
    {{ 1.0f,  1.0f,  1.0f},   {0.0f,  1.0f,  0.0f},   {1.0f, 1.0f}},
    {{-1.0f,  1.0f,  1.0f},   {0.0f,  1.0f,  0.0f},   {0.0f, 1.0f}},
};

unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,        // front
    6, 5, 4, 4, 7, 6,        // back
    8, 9,10,10,11, 8,        // left
    14,13,12,12,15,14,        // right
    16,17,18,18,19,16,        // bottom
    22,21,20,20,23,22         // top
}; 

unsigned int indicesSkyBox[] = {
    2, 1, 0, 0, 3, 2,        // front
    4, 5, 6, 6, 7, 4,        // back
    10, 9, 8, 8,11,10,        // left
    12,13,14,14,15,12,        // right
    18,17,16,16,19,18,        // bottom
    20,21,22,22,23,20         // top
};

Vertex quadVertices[] = {
    // Position               // Normal              // Tex Coords

    {{-1.0f, -1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {0.0f, 0.0f}},  // Bottom-left
    {{ 1.0f, -1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {1.0f, 0.0f}},  // Bottom-right
    {{ 1.0f,  1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {1.0f, 1.0f}},  // Top-right
    {{-1.0f,  1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {0.0f, 1.0f}},  // Top-left
};

float quadVerticess[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
};

#endif