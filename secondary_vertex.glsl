#version 330

layout(location = 0) in vec2 pos;
noperspective out vec2 coord;
noperspective out vec3 ray_direction;

uniform mat3 ray_xfrm;

void main()
{
    coord = pos.xy * 0.5 + vec2(0.5);
    ray_direction = ray_xfrm * vec3(pos, -1.0);
    //ray_direction = vec3(pos, -1.0);
    gl_Position = vec4(pos, 0.0, 1.0);
}

