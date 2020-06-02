#version 330

in vec3 pos;
in vec3 obj_pos;
in vec3 norm;
in vec4 shadow_clip[4];
in vec2 tc;
in vec3 proj_delta0_out;
in vec3 proj_delta1_out;

uniform mat4 modelview;
uniform sampler2D noise_tex;
uniform sampler2DArrayShadow shadowbuffers;
uniform sampler2D paper_texture;
uniform float material;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

const vec2 scale = vec2(1.0/1024.0);
      
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

    vec3 n = normalize(norm.xyz);
    
    float s= 1.0;

    vec3 pn = normalize(texture2D(noise_tex, tc * s).rgb - vec3(0.5) + (texture2D(noise_tex, tc * s * 2.0).rgb - vec3(0.5)) * 0.5);
    vec3 paper_normal = texture(paper_texture,tc / 16.0).rgb * 2.0 - vec3(1.0);
    
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
        
    if(material==1.0)
        n += normalize(texture2D(noise_tex, obj_pos.xz * 0.1).rgb - vec3(0.5)) * 6.0;
        
    position.w = sh;
    normal.rgb = normalize(n + pn * 0.05 + paper_normal * 0.1) * 0.5 + vec3(0.5);
    position.rgb = pos.rgb;
    normal.w = material;

    screenspace_velocity.xy = (proj_delta0_out.xy / proj_delta0_out.z - proj_delta1_out.xy / proj_delta1_out.z + vec2(1.0)) * 0.5;
    screenspace_velocity.z = 0.0;
}

