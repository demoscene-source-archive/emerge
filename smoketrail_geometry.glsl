#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 modelview,projection;

flat out vec3 norm;
out vec3 pos;

void main()
{
    vec3 face_normal=normalize(mat3(modelview)*cross(gl_in[1].gl_Position.xyz-gl_in[0].gl_Position.xyz,gl_in[2].gl_Position.xyz-gl_in[0].gl_Position.xyz));
    norm=face_normal;
    pos=(modelview*gl_in[0].gl_Position).xyz;
    gl_Position=projection*vec4(pos,1.0);
    EmitVertex();
    norm=face_normal;
    pos=(modelview*gl_in[1].gl_Position).xyz;
    gl_Position=projection*vec4(pos,1.0);
    EmitVertex();
    norm=face_normal;
    pos=(modelview*gl_in[2].gl_Position).xyz;
    gl_Position=projection*vec4(pos,1.0);
    EmitVertex();
    EndPrimitive();
}
