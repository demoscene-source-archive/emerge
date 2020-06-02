#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 coord_in;
noperspective out vec2 coord;

void main()
{
    coord = coord_in;
    gl_Position = vec4(pos, 0.0, 1.0);
}

