#version 330

uniform sampler2D chunk_tex;
uniform sampler2D noise_tex;
uniform float time;
uniform mat4 world_to_sdf;

noperspective in vec2 coord;

layout(location = 0) out vec3 colour;

vec2 rotate(float a,vec2 v)
{
   return vec2(cos(a)*v.x+sin(a)*v.y,cos(a)*v.y-sin(a)*v.x);
}

float wedge(vec4 w,vec2 uv)
{
   vec4 c=vec4(rotate(w.x,uv).x, rotate(w.y,-uv).x, length(uv)-w.z, -length(uv)+w.w);
   return length(max(vec4(0.0),c));
}

float logoDistance(vec3 p)
{
   vec2 uv = p.xy;
   float d=1e3;
   
   d=min(d,max(length(uv)-0.84, -length(uv)+0.72));
   d=min(d,max(length(uv)-0.52, -length(uv)+0.45));
   d=min(d,max(length(uv)-0.17, -length(uv)+0.08));

   d=min(d,wedge(vec4(1.54,0.53,0.91,0.72),uv));
   d=min(d,wedge(vec4(0.63,0.78,0.91,0.72),uv));

   d=min(d,wedge(vec4(0.61,1.675,0.72,0.64),uv));
   d=min(d,wedge(vec4(2.7,3.11,0.72,0.64),uv));
   d=min(d,wedge(vec4(3.45,3.65,0.72,0.64),uv));
   d=min(d,wedge(vec4(4.71,5.02,0.72,0.64),uv));
   d=min(d,wedge(vec4(5.3,5.51,0.72,0.64),uv));
   d=min(d,wedge(vec4(5.96,6.43,0.72,0.64),uv));

   d=min(d,wedge(vec4(3.2,1.27,0.45,0.35),uv));
   d=min(d,wedge(vec4(1.3,2.3,0.45,0.35),uv));
   d=min(d,wedge(vec4(2.58,4.2,0.45,0.35),uv));
   d=min(d,wedge(vec4(3.2,3.95,0.35,0.25),uv));
   d=min(d,wedge(vec4(5.2,5.93,0.35,0.25),uv));
   d=min(d,wedge(vec4(7.9,8.15,0.35,0.25),uv));
   d=min(d,wedge(vec4(0.2,1.16,0.32,0.17),uv));

   return max(max(d,p.z-1.0),-(p.z+1.0));
}

void main()
{
    vec3 nf=texture(noise_tex,coord).rgb;
    vec3 obj_pos = texture(chunk_tex,coord).rgb;
    for(int i = 0; i < 4; i += 1)
    {
        float t = max(0.0,-nf.g*2.0+ time*0.5+0.02*(obj_pos.z-100)+obj_pos.y*0.1 - 2.0);
        vec3 v = vec3(
                    0.0,
                    8.0*nf.b * -0.2 * min(2.0, t)-0.03,
                    min(8.0, max(0.0,-nf.g*2.0+ time*1.0*(0.5+cos(coord.x))+0.02*(obj_pos.z-100)+obj_pos.y*0.1 - 2.0))) * 0.25;
        vec3 lp=(world_to_sdf * vec4(obj_pos, 1.0)).xyz;
        float d = logoDistance(lp);
        if(d<0.01)
        {
            vec2 e = vec2(1e-2, 0.0);
            vec3 n = normalize(transpose(mat3(world_to_sdf)) * vec3(logoDistance(lp+e.xyy) - d, logoDistance(lp+e.yxy) - d, logoDistance(lp+e.yyx) - d));
            vec3 r = v - n * dot(n, v) * 1.0;
            v = r + n * 0.01;
        }
        obj_pos += v;
    }
    colour = obj_pos;
}

