#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexOut
{
    vec3 ro, rd;
    
} v_out[3];

out GeometryOut
{
    vec3 ro, rd;
    vec3 normal;

} g_out;

in vec3 proj_delta0[3];
in vec3 proj_delta1[3];

out vec3 proj_delta0_out;
out vec3 proj_delta1_out;

uniform mat4 modelview;
uniform mat4 projection;

uniform mat4 surface_to_ground;

vec3 face_normal;

void emit(int i,vec3 v)
{
    proj_delta0_out = proj_delta0[i];
    proj_delta1_out = proj_delta1[i];
    g_out.ro = v_out[i].ro;
    g_out.rd = v_out[i].rd;
    g_out.normal = face_normal;
    gl_Position = projection * modelview * vec4(v, 1.0);
    EmitVertex();
}

void main()
{
    face_normal = normalize(mat3(surface_to_ground) * cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz));

    emit(0,gl_in[0].gl_Position.xyz);
    emit(1,gl_in[1].gl_Position.xyz);
    emit(2,gl_in[2].gl_Position.xyz);
}