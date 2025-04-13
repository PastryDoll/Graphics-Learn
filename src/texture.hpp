#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb/stb_image.h"
#include "../thirdparty/glm/glm.hpp"
#include "../thirdparty/glm/gtc/type_ptr.hpp"


#include <glad/glad.h>

enum Texture_Types {
    TEXTURE_DIFFUSE,
    TEXTURE_SPECULAR,
    TEXTURE_NORMAL,
    TEXTURE_HEIGHT,
    TEXTURE_TYPES_MAX
};

const char * g_texture_types_str[TEXTURE_TYPES_MAX] = {"texture_diffuse", "texture_specular", "texture_normal", "texture_height"};

struct Texture {
    unsigned int ID;
    int type;
    char path[512];
};

Texture createTextureFromFile(const char * path, const char *directory, int texture_type, const bool flip_uv)
{

    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s", directory, path);

    Texture texture = {0};
    glGenTextures(1, &texture.ID);
    texture.type = texture_type;
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flip_uv);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(!flip_uv);

    printf("Loading image with um of channes %u\n",nrChannels);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrChannels == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;

        }
        else if (nrChannels == 3)
        {
            if (texture_type == TEXTURE_DIFFUSE) {internalFormat = GL_SRGB8;}
            else {internalFormat = GL_RGB8;}
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            if (texture_type == TEXTURE_DIFFUSE) {internalFormat = GL_SRGB8_ALPHA8;}
            else {internalFormat = GL_RGBA8;}
            dataFormat = GL_RGBA;
        }
        else
        {
            printf("Error: Unsupported number of channels: %d\n", nrChannels);
            stbi_image_free(data);
            return texture; 
        }
        glBindTexture(GL_TEXTURE_2D, texture.ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Row alignment
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);

    return texture;
}

unsigned int loadCubemap(const char *faces[6]) 
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            printf("Cubemap texture failed to load at path: %s\n", faces[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

Texture createSingleColorTexture(int type, glm::u8vec3 color) {

    Texture texture = {0};
    glGenTextures(1, &texture.ID);
    texture.type = type;
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, glm::value_ptr(color));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

#endif
