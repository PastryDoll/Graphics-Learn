#version 460 core

const int LIGHT_TYPE_DIRECTIONAL = 0;
const int LIGHT_TYPE_POSITIONAL  = 1;
const int LIGHT_TYPE_SPOTLIGHT   = 2;

struct Material {
    sampler2D texture_diffuse1;
	sampler2D texture_specular1;
    float     shininess;
}; 

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos; 

uniform Material material;
uniform vec3 viewPos;

#define NR_POINT_LIGHTS 4  // TODO fix this needs to be dynamic
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec4 diffuseTextureColor, vec4 specularTextureColor)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    if (diff == 0.0) {spec = 0.0;}

    vec3 ambient = light.ambient * vec3(diffuseTextureColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseTextureColor);
    vec3 specular = light.specular * spec * vec3(specularTextureColor);

    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 diffuseTextureColor, vec4 specularTextureColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    if (diff == 0.0) {spec = 0.0;}

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // calculate lighting
    vec3 ambient = light.ambient * vec3(diffuseTextureColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseTextureColor);
    vec3 specular = light.specular * spec * vec3(specularTextureColor);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 diffuseTextureColor, vec4 specularTextureColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    if (diff == 0.0) {spec = 0.0;}

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // calculate lighting
    vec3 ambient = light.ambient * vec3(diffuseTextureColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseTextureColor);
    vec3 specular = light.specular * spec * vec3(specularTextureColor);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec4 diffuseTextureColor = texture(material.texture_diffuse1, TexCoord);
    vec4 specularTextureColor = texture(material.texture_specular1, TexCoord);

    float alpha = diffuseTextureColor.a;

    vec3 result = CalcDirLight(dirLight, norm, viewDir, diffuseTextureColor, specularTextureColor);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, diffuseTextureColor, specularTextureColor);    

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, diffuseTextureColor, specularTextureColor);    

    FragColor = vec4(result, 1.0);
}