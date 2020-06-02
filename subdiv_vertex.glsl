#version 330

uniform mat4 projection;
uniform mat4 modelview;

layout(location = 0) in vec4 vertex;

void main()
{
    gl_Position = vertex;
}

