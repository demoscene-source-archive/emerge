#version 330

layout(points) in;
layout(points, max_vertices=3) out;

out float c;

void main()
{
    float s = 0.01;

    gl_Position.xy = gl_in[0].gl_Position.xy * 2.0 - vec2(1.0);
    gl_Layer = int(gl_in[0].gl_Position.z * 128.0);
    gl_Position.w = 1.0;
    c = 0.2 * s;
    EmitVertex();

    gl_Position.xy = gl_in[0].gl_Position.xy * 2.0 - vec2(1.0);
    gl_Layer = int(gl_in[0].gl_Position.z * 128.0) + 2;
    gl_Position.w = 1.0;
    c = 0.8 * s;
    EmitVertex();

    gl_Position.xy = gl_in[0].gl_Position.xy * 2.0 - vec2(1.0);
    gl_Layer = int(gl_in[0].gl_Position.z * 128.0) + 2;
    gl_Position.w = 1.0;
    c = 0.3 * s;
    EmitVertex();
}
