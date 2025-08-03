#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;
uniform float scale;
uniform vec2 centerPoint;


void main()
{
    vec2 centeredTexCoord = TexCoord - centerPoint;
    vec2 scaledTexCoord = centeredTexCoord * scale;
    vec2 finalTexCoord = scaledTexCoord + centerPoint;

    FragColor = texture(texture1, finalTexCoord);
    
}