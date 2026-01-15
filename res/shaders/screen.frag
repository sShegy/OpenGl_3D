#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effectType;

void main()
{
    vec4 color = texture(screenTexture, TexCoords);

    if(effectType == 1)
    {
        FragColor = vec4(1.0 - color.rgb, 1.0);
    }
    else if(effectType == 2)
    {
        color.rgb *=  1.0 - smoothstep(0.4, 1.5, length(TexCoords - 0.5));
        FragColor = color;
    }
    else if(effectType == 3)
    {
        float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
        FragColor = vec4(avg, avg, avg, 1.0);
    }
    else
    {
        FragColor = color;
    }
}