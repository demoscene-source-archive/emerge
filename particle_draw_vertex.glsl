#version 330

uniform mat4 modelview;
uniform mat4 projection;

uniform sampler2D particle_tex;
uniform sampler2D particle_tex2;
uniform sampler3D shadow_texture0;
uniform sampler3D collision_map;
uniform sampler3D shadow_texture1;
uniform float tween;
uniform mat4 world_to_voxel;
uniform vec3 tsiz;
uniform float shadow_voxel_tween;
uniform float vertical_spread;
uniform float fade;

layout(location = 0) in vec2 coord;

flat out vec4 pcol;

float sampleShadowVoxels(vec3 p)
{
    return mix(texture(shadow_texture0, p).r, texture(shadow_texture1, p).r, shadow_voxel_tween);
}

void main()
{
    vec3 world_pos = mix(texture(particle_tex2, coord).rgb, texture(particle_tex, coord).rgb, tween);
    world_pos.y += cos(gl_VertexID*2.1) * 0.2 * vertical_spread;
    vec3 cc = (world_to_voxel * vec4(world_pos, 1.0)).xyz;

    float sparkle = pow(0.5+0.5*sin(float(gl_VertexID)*17.0),256.0);
    
    pcol.rgb = mix(vec3(0.2, 0.2, 0.05) * mix(0.8, 4.1, 0.5+0.5*cos(gl_VertexID)), vec3(2.0), sparkle) * 1.4;
    pcol.rgb *= vec3(pow(max(0.2,1.0 - 1.0 * texture(collision_map, cc+vec3(0.0,0.0,-1.5)/tsiz).r), 2.0));
    pcol.rgb *= vec3(pow(max(0.2,1.0 - 1.8 * sampleShadowVoxels(cc+vec3(-1.0,0.0,-1.2)/tsiz)), 1.5));
    pcol.a = step(mod(float(gl_VertexID), 1101.0), 1.0) * 10.0;
    
    pcol *= fade;
    
    gl_Position = projection * modelview * vec4(world_pos, 1.0);
}
