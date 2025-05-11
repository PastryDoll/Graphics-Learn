#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform bool hdr;
uniform float exposure;

void main()
{             
    vec3 hdrColor = texture(texture1, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(hdr)
    {
        if(bloom)
        {
            hdrColor += bloomColor;
        }
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        FragColor = vec4(result, 1.0);
    }
    else
    {
        FragColor = vec4(hdrColor, 1.0);
    }
}