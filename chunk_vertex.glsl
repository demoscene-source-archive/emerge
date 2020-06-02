#version 330

uniform mat4 projection;
uniform mat4 modelview;

uniform sampler2D chunk_texture0;
uniform sampler2D chunk_texture1;

uniform float tween;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texcoord;

out vec3 proj_delta0;
out vec3 proj_delta1;
out vec3 vertex_obj_pos;

void main()
{
    vec4 chunk_offset = vec4(mix(texture(chunk_texture0, texcoord).rgb, texture(chunk_texture1, texcoord).rgb, tween), 0.0);
    vec4 v = vertex + chunk_offset;
    gl_Position = modelview * v;
    
    vec4 p0 = modelview * projection * v;
    
    proj_delta0 = vec3(p0.xy, p0.w);
    proj_delta1 = vec3(p0.xy, p0.w);
    
    vertex_obj_pos = v.xyz;
}

