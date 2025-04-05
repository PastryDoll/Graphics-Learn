#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#else
#include <errno.h>  
#endif

struct Shader
{
    unsigned int ID;
};
  

char* readFile(const char* filePath) {
    FILE* file;
#ifdef _WIN32
    errno_t err = fopen_s(&file, filePath, "rb");
    if (err != 0 || file == NULL) {
        fprintf(stderr, "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: %s (Error code: %d)\n", filePath, err);
        return NULL;
    }
#else
    file = fopen(filePath, "rb");
    if (file == NULL) {
        fprintf(stderr, "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: %s (Error: %s)\n", filePath, strerror(errno));
        return NULL;
    }
#endif

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize < 0 || fileSize > SIZE_MAX - 1) {
        fclose(file);
        fprintf(stderr, "ERROR::SHADER::FILE_SIZE_OVERFLOW\n");
        return NULL;
    }

    char* buffer = (char*)malloc((size_t)fileSize + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "ERROR::SHADER::MEMORY_ALLOCATION_FAILED\n");
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        fprintf(stderr, "ERROR::SHADER::FILE_READ_FAILED: %s\n", filePath);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[fileSize] = '\0';
    fclose(file);
    return buffer;
}

void checkCompileErrors(unsigned int shader, const char* type) {
    int success;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n", type, infoLog);
        }
    }
}

Shader createShaderFromFile(const char* vertexPath, const char* fragmentPath) {
    Shader shader = {0};
    char* vertexCode = readFile(vertexPath);
    char* fragmentCode = readFile(fragmentPath);

    if (!vertexCode || !fragmentCode) {
        fprintf(stderr, "ERROR::SHADER::FAILED_TO_READ_SHADER_FILES\n");
        if (vertexCode) free(vertexCode);
        if (fragmentCode) free(fragmentCode);
        return shader; 
    }

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertexCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragmentCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    shader.ID = glCreateProgram();
    glAttachShader(shader.ID, vertex);
    glAttachShader(shader.ID, fragment);
    glLinkProgram(shader.ID);
    checkCompileErrors(shader.ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(vertexCode);
    free(fragmentCode);

    return shader;
}

void useShader(Shader shader) {
    glUseProgram(shader.ID);
}

void setBool(Shader shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader.ID, name), (int)value);
}

void setInt(Shader shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader.ID, name), value);
}

void setFloat(Shader shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader.ID, name), value);
}

void setVec3(Shader shader, const char* name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(shader.ID, name), x, y, z);
}

void setMat4(Shader shader, const char* name, const float* mat) {
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, name), 1, GL_FALSE, mat);
}

void deleteShader(Shader shader) {
    glDeleteProgram(shader.ID);
}

#endif