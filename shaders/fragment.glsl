#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

const int LIGHT_TYPE_DIRECTIONAL = 0;
const int LIGHT_TYPE_POSITIONAL  = 1;
const int LIGHT_TYPE_SPOTLIGHT   = 2;

struct Material {
    sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
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

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform Material material;
uniform sampler2D shadowMap;
uniform samplerCube cubeShadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;
uniform bool useNormalMap;

#define NR_POINT_LIGHTS 1  // TODO fix this needs to be dynamic
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform DirLight dirLight;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation()
{
    vec3 fragToLight = fs_in.FragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fs_in.FragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(cubeShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;  
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);
        
    return shadow;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec4 diffuseTextureColor, vec4 specularTextureColor)
{
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    if (diff == 0.0) {spec = 0.0;}

    // attenuation
    float distance = length(lightPos - fs_in.FragPos);
    float attenuation = 1.0 /  (1.0 * (distance * distance));    
    // calculate lighting
    vec3 ambient = light.ambient * vec3(diffuseTextureColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseTextureColor);
    vec3 specular = light.specular * spec * vec3(specularTextureColor);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = shadows ? ShadowCalculation() : 0.0;   

    return (ambient + (1 - shadow) * (diffuse + specular));
    // return vec3(shadow);
}


void main()
{           
    vec4 diffuseTextureColor = texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 specularTextureColor = texture(material.texture_specular1, fs_in.TexCoords);


    vec3 normal;

    if (useNormalMap) {
        normal = texture(material.texture_normal1, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0); // Transform from [0,1] to [-1,1]

    } else {

        normal = normalize(fs_in.Normal);

    }

    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], normal, viewDir, diffuseTextureColor, specularTextureColor);
    }

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}