#version 330

uniform mat4 object_to_clip;
uniform mat4 previous_object_to_clip;
uniform mat4 modelview;
uniform mat4 projection;

layout(location = 0) in vec4 vertex;

out vec3 proj_delta0;
out vec3 proj_delta1;
out vec3 pos;

void main()
{
    vec4 v = modelview * vertex;
    pos = v.xyz;
    gl_Position = projection * v;
    
    vec4 p0 = object_to_clip * vertex;
    vec4 p1 = previous_object_to_clip * vertex;
    
    proj_delta0 = vec3(p0.xy, p0.w);
    proj_delta1 = vec3(p1.xy, p1.w);
}

