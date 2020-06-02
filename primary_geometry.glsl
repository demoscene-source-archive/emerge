#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vertex_normal[3];
in vec3 object_normal[3];
in vec3 proj_delta0[3];
in vec3 proj_delta1[3];
in vec3 vertex_obj_pos[3];

out vec3 pos;
out vec3 obj_pos;
out vec3 norm;
out vec4 shadow_clip[5];
out vec2 tc;
out vec3 proj_delta0_out;
out vec3 proj_delta1_out;

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 world_to_shadow[5];
uniform mat4 object_to_world;

uniform float normal_offset_scale;

void emit(vec3 v)
{
    vec3 n = normalize(norm.xyz);
    vec3 on = mat3(object_to_world) * inverse(mat3(modelview)) * n;

    gl_Position = projection * vec4(v, 1.0);    

    for(int i = 0; i < 5; i += 1)
        shadow_clip[i] = world_to_shadow[i] * vec4(obj_pos + on * normal_offset_scale, 1.0);
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
    proj_delta0_out = proj_delta0[0];
    proj_delta1_out = proj_delta1[0];
    obj_pos = vertex_obj_pos[0];
    emit(gl_in[0].gl_Position.xyz);

    norm = face_normal;
    proj_delta0_out = proj_delta0[1];
    proj_delta1_out = proj_delta1[1];
    obj_pos = vertex_obj_pos[1];
    emit(gl_in[1].gl_Position.xyz);

    norm = face_normal;
    proj_delta0_out = proj_delta0[2];
    proj_delta1_out = proj_delta1[2];
    obj_pos = vertex_obj_pos[2];
    emit(gl_in[2].gl_Position.xyz);

    EndPrimitive();
}
