#version 330

uniform isampler2D node0; // 0
uniform isampler2D node1; // 1
uniform sampler2D edge0; // 2
uniform sampler2D edge1; // 3
uniform usampler2D triangle; // 4

uniform sampler2D noise_tex;
uniform mat4 ground_to_view;
uniform float material;
uniform float spread_level;

in GeometryOut
{
    vec3 ro, rd;
    vec3 normal;

} g_out;

in vec3 proj_delta0_out;
in vec3 proj_delta1_out;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec3 screenspace_velocity;

vec2 boxInterval(vec3 ro, vec3 rd, vec3 size)
{
    vec3 slabs0 = (+size - ro) / rd;
    vec3 slabs1 = (-size - ro) / rd;

    vec3 mins = min(slabs0, slabs1);
    vec3 maxs = max(slabs0, slabs1);

    return vec2(max(max(mins.x, mins.y), mins.z), min(min(maxs.x, maxs.y), maxs.z));
}

float trace(vec3 ro, vec3 rd, out vec3 out_norm)
{
    int node = 0;
    float hdist = 1e6;

    do
    {
        ivec2 node_coord = ivec2(int(node & 0xffff), int(node >> 16));
        
        ivec4 n0 = texelFetch(node0, node_coord, 0);
        ivec4 n1 = texelFetch(node1, node_coord, 0);
    
        vec3 box_min = vec3(n0.x, n0.y, n0.z);
        vec3 box_max = vec3(n1.x, n1.y, n1.z);
    
        vec3 box_size = (box_max - box_min) * 0.5;
        vec3 box_center = (box_min + box_max) * 0.5;
        
        vec2 i = boxInterval(ro - box_center, rd, box_size);
        
        node = n0.w;

        if(i.x < 0.0 && i.x < i.y && -i.y < hdist)
        {
            if((n1.w & 1) > 0)
            {
                n1.w >>= 1;
            
                ivec2 coord = ivec2(int(n1.w & 0xffff), int(n1.w >> 16));
                
                uvec4 tri = texelFetch(triangle, coord, 0);


                coord = ivec2(int(tri.x & 0xffU), int(tri.x >> 8U));
                vec3 p0a = texelFetch(edge0, coord, 0).xyz;
                vec3 p0b = texelFetch(edge1, coord, 0).xyz;

                coord = ivec2(int(tri.y & 0xffU), int(tri.y >> 8U));
                vec3 p1a = texelFetch(edge0, coord, 0).xyz;
                vec3 p1b = texelFetch(edge1, coord, 0).xyz;

                coord = ivec2(int(tri.z & 0xffU), int(tri.z >> 8U));
                vec3 p2a = texelFetch(edge0, coord, 0).xyz;
                vec3 p2b = texelFetch(edge1, coord, 0).xyz;
                
                vec3 p0 = (tri.w & 1U) > 0U ? p0b : p0a;
                vec3 p1 = (tri.w & 2U) > 0U ? p1b : p1a;
                vec3 p2 = (tri.w & 4U) > 0U ? p2b : p2a;


                vec3 norm = cross(p2 - p0, p1 - p0);
                float t = dot(ro - p0, norm) / dot(rd, norm);
                if(t > 0.0)
                {
                    vec3 hp = ro - rd * t;

                    float b0 = length(cross(p0 - hp, p1 - hp));
                    float b1 = length(cross(p1 - hp, p2 - hp));
                    float b2 = length(cross(p2 - hp, p0 - hp));
                    
                    if((b0 + b1 + b2) < length(norm) + 1e-2 && t < hdist)
                    {
                         hdist = t;
                         out_norm = norm;
                    }
                }

            }
            else
            {
                node = n1.w >> 1;
            }
        }

               
    } while(node > 0);
    
    return hdist;
}

void main()
{
    float n = floor(mod(gl_FragCoord.x + gl_FragCoord.y, 2.0));//  texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy,textureSize(noise_tex, 0))), 0).b;
    float f = pow(1.0 - dot(-normalize(g_out.rd), normalize(g_out.normal)), 4.0);
    
    vec3 norm;
    vec3 rd0 = refract(normalize(g_out.rd), normalize(g_out.normal), 1.0 / 1.333);
    vec3 rd1 = reflect(normalize(g_out.rd), normalize(g_out.normal));

    float spread = step(length(g_out.ro.xy),spread_level-texture(noise_tex,g_out.ro.xy*0.001).g*100.0-texture(noise_tex,g_out.ro.xy*0.00003).g*100.0);
    
    vec3 rd = rd1;
    
    if(spread > 0.5)
    {
        normal.w = 1.0;
        position.rgb = (ground_to_view * vec4(g_out.ro, 1.0)).xyz;
        position.w = 1.0;
    }
    else
    {
        rd = ((n + 0.5 - f) > 0.5) ? rd0 : rd1;
        float hdist = trace(g_out.ro, rd, norm);
        normal.w = hdist > 1e5 ?  0.0 : material;
        position.rgb = (ground_to_view * vec4(g_out.ro + rd * hdist, 1.0)).xyz;
        position.w = 0.7;
    }

    vec3 nv = mat3(ground_to_view) * -norm.xyz;
    nv += normalize(texture2D(noise_tex, g_out.ro.xy * 0.0025).rgb - vec3(0.5)) * 6.0;
    
    normal.rgb = normalize(nv) * 0.5 + vec3(0.5);
    
    screenspace_velocity.xy = (proj_delta0_out.xy / proj_delta0_out.z - proj_delta1_out.xy / proj_delta1_out.z + vec2(1.0)) * 0.5;
    screenspace_velocity.z = 0.0;
}

