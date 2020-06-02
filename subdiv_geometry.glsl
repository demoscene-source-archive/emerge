#version 330

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 3) out;

flat out float mat;
out vec3 pos;
flat out vec3 norm;
out vec4 shadow_clip[4];

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 world_to_shadow[4];
uniform float material;
uniform float material2;
uniform vec3 spin_pivot;
uniform float time;

vec3 face_normal;

float t = time + (gl_in[0].gl_Position.y - 36.0);

float angle0 = t;
mat3 rotm0 = mat3(cos(angle0), 0.0, sin(angle0),
                 0.0, 1.0, 0.0,
                 -sin(angle0), 0.0, cos(angle0));
                 
float angle1 = max(0.0,-(t - 12.0));
mat3 rotm1 = mat3(cos(angle1), 0.0, sin(angle1),
                 0.0, 1.0, 0.0,
                 -sin(angle1), 0.0, cos(angle1));
                 
void emit(vec3 v)
{
    mat=mix(material,material2,step(7.0,t));
    
    gl_Position = projection * modelview * vec4(v, 1.0);    

    for(int i = 0; i < 4; i += 1)
        shadow_clip[i] = world_to_shadow[i] * vec4(v + face_normal * 0.06, 1.0);
        
    pos = (modelview * vec4(v, 1.0)).xyz;
    norm = normalize(mat3(modelview) * face_normal);
    
    EmitVertex();
}

vec3 deform(vec3 p0, vec3 p1)
{
    vec3 p2 = rotm0 * ((p0 + vec3(0.0, 1.0, 0.0)) - spin_pivot) + spin_pivot + vec3(0.0,2.0,0.0);
    vec3 p3 = rotm1 * (p1 - spin_pivot) + spin_pivot;
    
    return mix(p0, mix(p2, p3, smoothstep(5.0, 9.0, t + (p1.y - 36.0))), smoothstep(0.0, 4.0, t + (p1.y - 36.0)));
}

void main()
{
    vec3 points[3];

    points[0] = deform(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz);
    points[1] = deform(gl_in[2].gl_Position.xyz, gl_in[3].gl_Position.xyz);
    points[2] = deform(gl_in[4].gl_Position.xyz, gl_in[5].gl_Position.xyz);

    face_normal = normalize(cross(points[1] - points[0], points[2] - points[0]));
    
    emit(points[0]);
    emit(points[1]);
    emit(points[2]);

    EndPrimitive();
}
