#ifndef MESH_H
#define MESH_H
#include "../thirdparty/glm/glm.hpp"
#include "texture.hpp"
#include "shader.hpp"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
} ;

static void setupMesh(struct Mesh* mesh); 
struct Mesh {
    Vertex* vertices;
    unsigned int* indices;
    Texture* textures;

    unsigned int numVertices;
    unsigned int numIndices;
    unsigned int numTextures;

    unsigned int VAO, VBO, EBO;

    Mesh(Vertex* vertices, unsigned int numVertices,
        unsigned int* indices, unsigned int numIndices,
        Texture* textures, unsigned int numTextures)
   {
       this->vertices = vertices;
       this->numVertices = numVertices;

       this->indices = indices;
       this->numIndices = numIndices;

       this->textures = textures;
       this->numTextures = numTextures;

       setupMesh(this);
   }

};

static void setupMesh(Mesh* mesh) {
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void activateMesh(Mesh* mesh, Shader* shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    char uniformName[128];

    for (unsigned int i = 0; i < mesh->numTextures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        const char* name = g_texture_types_str[mesh->textures[i].type];
        char number[10];

        if (strcmp(name, "texture_diffuse") == 0) {
            snprintf(number, sizeof(number), "%d", diffuseNr++);
        } else if (strcmp(name, "texture_specular") == 0) {
            snprintf(number, sizeof(number), "%d", specularNr++);
            setFloat(*shader, "material.shininess", 32.0f);

        }
        snprintf(uniformName, sizeof(uniformName), "material.%s%s", name, number);
        printf("Uniform: %s\n", uniformName);
        setInt(*shader, uniformName, i);

        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].ID);
    }
    glActiveTexture(GL_TEXTURE0);

}
void drawMesh(Mesh* mesh, Shader* shader) {
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#endif