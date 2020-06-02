#version 330

flat in float mat;
in vec3 pos;
flat in vec3 norm;
in vec4 shadow_clip[4];
uniform float time;

uniform mat4 modelview;
uniform mat4 object_to_world;
uniform sampler2D noise_tex;
uniform sampler2DArrayShadow shadowbuffers;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

const vec2 samples[8] = vec2[8](
 vec2(0.000000, -0.333333),
 vec2(-0.500000, 0.333333),
 vec2(0.500000, -0.777778),
 vec2(-0.750000, -0.111111),
 vec2(0.250000, 0.555556),
 vec2(-0.250000, -0.555556),
 vec2(0.750000, 0.111111),
 vec2(0.125000, -0.925926)
);
  
float blurredShadow(sampler2DArrayShadow sb,vec4 p)
{
    float sh=0.0;
    float scale = exp2(-p.z) / 512.0;
    for(int i = 0; i < samples.length(); i += 1)
    {
        sh += texture(sb, p + vec4(samples[i] * scale, vec2(0.0)));
    }
    return sh / float(samples.length());
}

void main()
{
    position.w = 1.0;

    float s= 1.0;
   
    float d = length(pos.rgb) + texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy,textureSize(noise_tex, 0))), 0).r * 10.0;
    float sh = 1.0;
  
    if(d > 80.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[3].xy / shadow_clip[3].w, 3.0, shadow_clip[3].z / shadow_clip[3].w));
    else if(d > 40.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[2].xy / shadow_clip[2].w, 2.0, shadow_clip[2].z / shadow_clip[2].w));
    else if(d > 20.0)
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[1].xy / shadow_clip[1].w, 1.0, shadow_clip[1].z / shadow_clip[1].w));
    else
        sh = blurredShadow(shadowbuffers, vec4(shadow_clip[0].xy / shadow_clip[0].w, 0.0, shadow_clip[0].z / shadow_clip[0].w));

    position.w = mix(sh,1.0,step(22.0,time));
    normal.rgb = faceforward(norm, pos.rgb, norm) * 0.5 + vec3(0.5);
    //normal.rgb = normalize(n) * 0.5 + vec3(0.5);
    position.rgb = pos.rgb;
    normal.w = mat;
    screenspace_velocity=vec3(0.5,0.5,0.0);
}

