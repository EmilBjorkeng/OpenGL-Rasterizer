#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
flat in int TexID;

uniform sampler2D textures[16];

void main()
{
    FragColor = texture(textures[TexID], TexCoord);
}