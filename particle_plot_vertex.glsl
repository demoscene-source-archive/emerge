#version 330

uniform sampler2D particle_tex;
uniform sampler2D particle_tex2;
uniform float tween;
uniform mat4 world_to_voxel;
uniform vec3 tsiz;

layout(location = 0) in vec2 coord;

void main()
{
    vec3 world_pos = mix(texture(particle_tex2, coord).rgb, texture(particle_tex, coord).rgb, tween);
    vec3 cc = (world_to_voxel * vec4(world_pos, 1.0)).xyz;
    gl_Position.xyz = cc+vec3(0.0,0.0,0.6)/tsiz;
    gl_Position.w = 1.0;
}
