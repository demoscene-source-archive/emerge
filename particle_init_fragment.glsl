#version 330

uniform sampler2D particle_tex;
uniform sampler3D collision_map;
uniform sampler2D noise_tex;
uniform float time;
uniform sampler1D velocity_map;
uniform float velocity_map_seconds;

noperspective in vec2 coord;

layout(location = 0) out vec3 colour;

void main()
{
    float t=max(0.0,time-(coord.x*2048.0+coord.y)/2048.0*3.0);

    vec3 vm = texture(velocity_map, t / velocity_map_seconds).rgb;
    vec3 world_pos = texture(particle_tex,coord).rgb;
    vec3 nf0 = (texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy,textureSize(noise_tex, 0))), 0).rgb - vec3(0.5)) * 2.0;
    vec3 nf1 = (texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy * 0.5,textureSize(noise_tex, 0))), 0).rgb - vec3(0.5)) * 2.0;    
    
    if(t<0.01)
    {
        colour.xyz = mix(mix(vec3(-34.73516, 33.78981, -2.89416), vec3(-35.3329, 34.5637, -3.0263), step(coord.y, 0.9)), vec3(-35.39828, 33.58353, -2.0913), step(coord.y, 0.05)) + nf0 * 0.01;
    }
    else
    {
        colour.xyz = world_pos + (nf1 * nf1 * sign(nf1) * 0.013 * mix(1.0, 0.5, min(step(0.9, coord.y), step(coord.y, 0.05))) + vec3(0.02,0.02,0.04 + 0.02 * (-1.0 + max(step(coord.y, 0.9), 2.0 * step(coord.y, 0.05)))  ) * 0.5) * max(0.0, 1.0 - t / 6.0) + vm * mix(0.9,1.0,nf1.z);
        colour.y+=cos(t*1.0+nf0.x*3.0)*0.015;
    }
}