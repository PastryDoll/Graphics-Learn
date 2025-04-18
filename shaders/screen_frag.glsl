#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool hdr;
uniform float exposure;

void main()
{             
    vec3 hdrColor = texture(texture1, TexCoords).rgb;
    if(hdr)
    {
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // also gamma correct while we're at it       
        FragColor = vec4(result, 1.0);
    }
    else
    {
        FragColor = vec4(hdrColor, 1.0);
    }
}