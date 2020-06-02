#version 330

in vec3 pos;
in vec3 obj_pos;
in vec3 norm;
in vec4 shadow_clip[5];
in vec2 tc;
in vec3 proj_delta0_out;
in vec3 proj_delta1_out;

uniform float material;
uniform mat4 modelview;
uniform mat4 object_to_world;
uniform sampler2D noise_tex;
uniform sampler2DArrayShadow shadowbuffers;
uniform sampler2D paper_texture;
uniform sampler3D shadow_voxel_texture0;
uniform sampler3D shadow_voxel_texture1;
uniform float shadow_voxel_tween;
uniform float shadow_voxel_scale;
uniform mat4 world_to_voxel;
uniform vec3 tsiz;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

float sampleShadowVoxels(vec3 world_pos)
{
    vec3 cc = (world_to_voxel * vec4(world_pos, 1.0)).xyz+vec3(0.0,0.0,-0.5)/tsiz;
    return mix(texture(shadow_voxel_texture0, cc).r, texture(shadow_voxel_texture1, cc).r, shadow_voxel_tween) * shadow_voxel_scale;
}

const vec2 samples[16] = vec2[16](
 vec2(0.000000, -0.333333),
 vec2(-0.500000, 0.333333),
 vec2(0.500000, -0.777778),
 vec2(-0.750000, -0.111111),
 vec2(0.250000, 0.555556),
 vec2(-0.250000, -0.555556),
 vec2(0.750000, 0.111111),
 vec2(0.125000, -0.925926),
 vec2(-0.375000, -0.259259),
 vec2(0.625000, 0.407407),
 vec2(-0.625000, -0.703704),
 vec2(0.375000, -0.037037),
 vec2(-0.125000, 0.629630),
 vec2(0.875000, -0.481481),
 vec2(-0.937500, 0.185185),
 vec2(0.062500, 0.851852)
);
  
float blurredShadow(sampler2DArrayShadow sb,vec4 p)
{
    //return texture(sb, p);
    
    float sh=0.0;
    float scale = exp2(-p.z) / 400.0;
    for(int i = 0; i < samples.length(); i += 1)
    {
        sh += texture(sb, p + vec4(samples[i] * scale, vec2(0.0)));
    }
    return sh / float(samples.length());
}

void main()
{
    position.w = 1.0;

    vec3 n = normalize(norm.xyz);
    
    float s= 1.0;

    vec3 pn = normalize(texture2D(noise_tex, tc * s).rgb - vec3(0.5) + (texture2D(noise_tex, tc * s * 2.0).rgb - vec3(0.5)) * 0.5);
    vec3 paper_normal = texture(paper_texture,tc / 16.0).rgb * 2.0 - vec3(1.0);
    
    float d = length(pos.rgb) + texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy,textureSize(noise_tex, 0))), 0).r * 10.0;
    float sh = 1.0;
  
  
    if(d > 160.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[4].xy / shadow_clip[4].w, 4.0, shadow_clip[4].z / shadow_clip[4].w));
    else if(d > 80.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[3].xy / shadow_clip[3].w, 3.0, shadow_clip[3].z / shadow_clip[3].w));
    else if(d > 40.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[2].xy / shadow_clip[2].w, 2.0, shadow_clip[2].z / shadow_clip[2].w));
    else if(d > 20.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[1].xy / shadow_clip[1].w, 1.0, shadow_clip[1].z / shadow_clip[1].w));
    else
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[0].xy / shadow_clip[0].w, 0.0, shadow_clip[0].z / shadow_clip[0].w));
        
    if(material>.999)
        n += normalize(texture2D(noise_tex, tc * 1.0).rgb - vec3(0.5));
        
    position.w = sh;
    normal.rgb = normalize(n + pn * 0.05 + paper_normal * 0.1) * 0.5 + vec3(0.5);
    //normal.rgb = normalize(n + pn * 0.05) * 0.5 + vec3(0.5);
    position.rgb = pos.rgb;
    normal.w = material;

    screenspace_velocity.xy = (proj_delta0_out.xy / proj_delta0_out.z - proj_delta1_out.xy / proj_delta1_out.z + vec2(1.0)) * 0.5;
    screenspace_velocity.z = sampleShadowVoxels(obj_pos);
}

