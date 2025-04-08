#ifndef LIGHT_H
#define LIGHT_H
#include "../thirdparty/glm/glm.hpp"
#include "../thirdparty/glm/gtc/type_ptr.hpp"

#include "shader.hpp"
#include <string>

typedef enum {
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT
} LightType;

typedef struct {
    LightType type;

    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
} Light;

void setLight(const char* name, const Light light, Shader shader) {
    setVec3(shader,(std::string(name) + ".ambient").c_str(), glm::value_ptr(light.ambient));
    setVec3(shader,(std::string(name) + ".diffuse").c_str(), glm::value_ptr(light.diffuse));
    setVec3(shader,(std::string(name) + ".specular").c_str(), glm::value_ptr(light.specular));

    switch (light.type) {
        case LIGHT_TYPE_DIRECTIONAL:
            setVec3(shader,(std::string(name) + ".direction").c_str(), glm::value_ptr(light.direction));
            break;

        case LIGHT_TYPE_POINT:
            setVec3(shader,(std::string(name) + ".position").c_str(), glm::value_ptr(light.position));
            setFloat(shader,(std::string(name) + ".constant").c_str(), light.constant);
            setFloat(shader,(std::string(name) + ".linear").c_str(), light.linear);
            setFloat(shader,(std::string(name) + ".quadratic").c_str(), light.quadratic);
            break;

        case LIGHT_TYPE_SPOT:
            setVec3(shader,(std::string(name) + ".position").c_str(), glm::value_ptr(light.position));
            setVec3(shader,(std::string(name) + ".direction").c_str(), glm::value_ptr(light.direction));
            setFloat(shader,(std::string(name) + ".constant").c_str(), light.constant);
            setFloat(shader,(std::string(name) + ".linear").c_str(), light.linear);
            setFloat(shader,(std::string(name) + ".quadratic").c_str(), light.quadratic);
            setFloat(shader,(std::string(name) + ".cutOff").c_str(), light.cutOff);
            setFloat(shader,(std::string(name) + ".outerCutOff").c_str(), light.outerCutOff);
            break;
    }
}
#endif
