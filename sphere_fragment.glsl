#version 330

in vec3 pos;
in vec3 norm;
in vec2 tc;

uniform vec3 colour;

layout(location = 0) out vec4 colour_out;

void main()
{
    colour_out.rgb=colour;
}

