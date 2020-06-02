#version 330

uniform float material;

in vec3 pos;
flat in vec3 norm;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

void main()
{
    position.rgb = pos;
    position.w = 1.0;
    normal.rgb = norm * 0.5 + vec3(0.5);
    normal.w = material;
    screenspace_velocity = vec3(0.5,0.5,0.0);
}
