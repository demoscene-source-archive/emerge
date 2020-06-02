#version 330

layout(location = 0) out vec4 colour;

flat in vec4 pcol;

void main()
{
    colour = pcol;
}

