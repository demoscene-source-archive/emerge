#version 330

uniform mat4 transformation;

layout(location = 0) in vec4 vertex;

void main()
{
    gl_Position = transformation * vertex;
}

