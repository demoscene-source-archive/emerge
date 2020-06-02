#version 330

uniform mat4 transformation;
uniform sampler2D chunk_texture0;
uniform sampler2D chunk_texture1;

uniform float tween;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texcoord;

void main()
{
    vec4 chunk_offset = vec4(mix(texture(chunk_texture0, texcoord).rgb, texture(chunk_texture1, texcoord).rgb, tween), 0.0);
    vec4 v = vertex + chunk_offset;
    gl_Position = transformation * v;
}

