#version 330

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 object_to_clip;
uniform mat4 previous_object_to_clip;
uniform mat4 object_to_world;
uniform mat4 normal_matrix;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;

out vec3 vertex_normal;
out vec3 proj_delta0;
out vec3 proj_delta1;
out vec3 vertex_obj_pos;

void main()
{
    gl_Position = modelview * vertex;
    
    vec4 p0 = object_to_clip * vertex;
    vec4 p1 = previous_object_to_clip * vertex;
    
    proj_delta0 = vec3(p0.xy, p0.w);
    proj_delta1 = vec3(p1.xy, p1.w);
    
    vertex_obj_pos = (object_to_world * vertex).xyz;
        
    vertex_normal = normalize(mat3(normal_matrix) * normal);
}

