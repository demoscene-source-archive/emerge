#version 330

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 object_to_clip;
uniform mat4 previous_object_to_clip;
uniform mat4 object_to_world;

uniform mat4 surface_to_ground;
uniform float time;

layout(location = 0) in vec4 vertex;

out vec3 proj_delta0;
out vec3 proj_delta1;

out VertexOut
{
    vec3 ro, rd;
    
} v_out;

vec3 displacement(vec3 v, float t)
{
    v.z += cos(v.x * 0.5 + t * 2.0 + v.y * 2.0) * 0.06;
    //v.z += (0.5 * cos(v.x * 20.0 + v.y * 15.0 + t * 1.5)) * -0.05;
    //v.x += sin(v.x * 10.0) * 0.1;
    return v;
}

void main()
{
    vec4 v = vertex;
    vec3 pv = displacement(v.xyz, time - 1.0 / 24.0);
    
    v.xyz = displacement(v.xyz, time);
    
    vec3 gv = (surface_to_ground * v).xyz;

    v_out.rd = normalize(-(gv.xyz - ((surface_to_ground * inverse(modelview))[3].xyz)));
    v_out.ro = gv;
    
    gl_Position = v;
    
    vec4 p0 = object_to_clip * v;
    vec4 p1 = previous_object_to_clip * vec4(pv, 1.0);
    
    proj_delta0 = vec3(p0.xy, p0.w);
    proj_delta1 = vec3(p1.xy, p1.w);
}

