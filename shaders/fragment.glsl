#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos; 

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 lightPos;  
uniform vec3 viewPos;

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos); 
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float ambientStrenght = 0.1;
	vec3 ambient = ambientStrenght * lightColor;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;  
	
	FragColor = vec4(ambient + diffuse + specular, 1.0) * mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
};