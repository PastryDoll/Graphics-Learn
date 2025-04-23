#version 460 core
out vec4 FragColor;


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

struct Material {
    sampler2D texture_diffuse1;
}; 

uniform Material material;

void main()
{             
    FragColor = texture(material.texture_diffuse1, fs_in.TexCoords);
}  