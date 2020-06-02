#version 330

layout(location = 0) out vec4 colour;

flat in vec3 normal;

void main()
{
    colour = vec4(0.5+0.5*normal.y);
}
