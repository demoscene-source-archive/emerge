#version 330

uniform sampler2D particle_tex;
uniform sampler3D collision_map;
uniform mat4 world_to_voxel;
uniform vec3 tsiz;

noperspective in vec2 coord;

layout(location = 0) out vec3 colour;

const float eps=1e-1;

float sampleCollisionMap(vec3 world_pos)
{
    vec3 cc = (world_to_voxel * vec4(world_pos, 1.0)).xyz;
    return texture(collision_map,cc+vec3(1.0,1.0,-1.5)/tsiz).r;
}

void main()
{
    vec3 obj_pos=texture(particle_tex,coord).rgb;
    for(int i=0;i<2;i+=1)
    {
        float d=sampleCollisionMap(obj_pos);
        vec3 v=vec3(0.000,-0.04,0.000)*0.25;
        float d0 = sampleCollisionMap(obj_pos+vec3(eps,0.0,0.0));
        float d1 = sampleCollisionMap(obj_pos+vec3(0.0,eps,0.0));
        float d2 = sampleCollisionMap(obj_pos+vec3(0.0,0.0,eps));
        if(d<0.25)
        {
            obj_pos+=v;
        }
        else
        {
            vec3 n=normalize(vec3(d0-d,d1-d,d2-d));
            vec3 r=v-n*dot(n,v);
            obj_pos+=r-n*0.02;
            obj_pos.y+=0.02;
        }
    }
    
    colour = obj_pos;
}

