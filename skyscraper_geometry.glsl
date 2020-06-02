#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vertex_obj_pos[3];

out vec3 pos;
out vec3 obj_pos;
out vec3 norm;
out vec2 tc;

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 object_to_world;

void emit(vec3 v)
{
    vec3 n = normalize(norm.xyz);
    vec3 on = inverse(mat3(modelview)) * n;

    gl_Position = projection * vec4(v, 1.0);    
    pos = v;
    
    tc=obj_pos.yz;
    
    vec3 na=abs(on);
    if(na.y > na.x && na.y > na.z)
        tc=obj_pos.xz;
    else if(na.z > na.x && na.z > na.y)
        tc=obj_pos.xy;
    
    EmitVertex();
}

void main()
{
    vec3 points[3];

    points[0] = gl_in[0].gl_Position.xyz;
    points[1] = gl_in[1].gl_Position.xyz;
    points[2] = gl_in[2].gl_Position.xyz;

    vec3 face_normal = normalize(cross(points[1] - points[0], points[2] - points[0]));
    
    norm = face_normal;
    obj_pos = vertex_obj_pos[0];
    emit(gl_in[0].gl_Position.xyz);

    norm = face_normal;
    obj_pos = vertex_obj_pos[1];
    emit(gl_in[1].gl_Position.xyz);

    norm = face_normal;
    obj_pos = vertex_obj_pos[2];
    emit(gl_in[2].gl_Position.xyz);

    EndPrimitive();
}
