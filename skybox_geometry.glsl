#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 modelview, projection;

flat out vec3 normal;

void main()
{
    vec3 fn = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz));
    normal = fn;
    gl_Position = projection * modelview * gl_in[0].gl_Position;
    EmitVertex();
    normal = fn;
    gl_Position = projection * modelview * gl_in[1].gl_Position;
    EmitVertex();
    normal = fn;
    gl_Position = projection * modelview * gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
