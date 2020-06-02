#version 330

layout(location = 0) in vec2 pos;
noperspective out vec2 coord;

void main()
{
    coord = pos.xy * 0.5 + vec2(0.5);
    gl_Position = vec4(pos, 0.0, 1.0);
}

