#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb/stb_image.h"

#include <glad/glad.h>

struct Texture {
    unsigned int ID;
};

Texture createTextureFromFile(const char * img_path)
{
    Texture texture = {0};
    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(img_path, &width, &height, &nrChannels, 0);
    printf("Loading image with um of channes %u\n",nrChannels);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrChannels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else
        {
            printf("Error: Unsupported number of channels: %d\n", nrChannels);
            stbi_image_free(data);
            return texture; 
        }
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
#endif
