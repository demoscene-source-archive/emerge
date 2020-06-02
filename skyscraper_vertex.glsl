#version 330

uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 object_to_world;

layout(location = 0) in vec4 vertex;

out vec3 vertex_obj_pos;

void main()
{
    gl_Position = modelview * vertex;
    vertex_obj_pos = vertex.xyz;
}

