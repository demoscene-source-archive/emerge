#version 330

uniform mat4 modelview;
uniform mat4 projection;

layout(location = 0) in vec4 vertex;

flat out vec3 origin;
out vec3 coord;

void main()
{
    origin = modelview[3].xyz;
    gl_Position = projection * modelview * vertex;
    coord = vec3(gl_Position.xy, gl_Position.w);
}

