#version 330

uniform sampler2D source_tex;
uniform vec2 direction;

layout(location = 0) out vec4 colour;

noperspective in vec2 coord;

void main()
{
    vec4 c = vec4(0.0);

    for(int i = 0; i < 7; ++i)
    {
        c += texture(source_tex, coord + direction * (float(i - 3)) / 3.0);
    }

    colour = c / 7.0;
}

