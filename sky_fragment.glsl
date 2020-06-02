#version 330

in vec3 pos;
in vec3 proj_delta0;
in vec3 proj_delta1;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

void main()
{
    position.w = 1.0;
    position.xyz = normalize(pos) * 1000.0;
    normal = vec4(0.0);

    screenspace_velocity.xy = (proj_delta0.xy / proj_delta0.z - proj_delta1.xy / proj_delta1.z + vec2(1.0)) * 0.5;
    screenspace_velocity.z = 0.0;
}

