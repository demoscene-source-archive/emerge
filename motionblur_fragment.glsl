#version 330

uniform sampler2D fb_final;
uniform sampler2D fb_position;
uniform sampler2D fb_velocity;

uniform float scale;

layout(location = 0) out vec4 colour;

noperspective in vec2 coord;

void main()
{
    vec4 c = vec4(0.0);
    vec2 v = (texture(fb_velocity, coord).rg * 2.0 - vec2(1.0)) * scale;
    for(int i = 0; i < 16; i += 1)
    {
        float t = float(i) / 16.0;
        c += texture(fb_final, coord + v * t * 0.5);
    }
    //colour.a = texture(fb_final, coord).a;
    //colour.rgb = c / 8.0;
    colour = c / 16.0;
}

