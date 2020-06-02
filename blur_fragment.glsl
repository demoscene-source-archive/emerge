#version 330

uniform sampler2D fb_final;
uniform sampler2D noise_tex;
uniform vec2 direction;
uniform float peripheral;

layout(location = 0) out vec4 colour;

noperspective in vec2 coord;

void main()
{
    vec4 c = vec4(0.0);
    float n = texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy, textureSize(noise_tex, 0))), 0).r;
    float x = mix(2.0, distance(coord, vec2(0.5)) * 2.0, peripheral);
    
    for(int i = 0; i < 7; ++i)
    {
        c += texture(fb_final, coord + x * direction * (float(i - 3) + n) / 3.0);
    }

    colour = c / 7.0;
}

