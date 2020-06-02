#version 330

in vec3 pos;
in vec3 obj_pos;
in vec3 norm;
in vec2 tc;

uniform sampler2D noise_tex;
uniform sampler2D noise_tex2;
uniform samplerCube cubemap_tex;

layout(location = 0) out vec4 colour;

void main()
{
    vec3 v=normalize(-pos);
    vec3 n=normalize(norm+texture(noise_tex,tc).rgb*0.05+texture(noise_tex2,tc).rgb*0.1);
    vec3 r=reflect(v,n);
    //colour.rgb=texture(cubemap_tex,v).rgb;
    colour.rgb=vec3(pow(texture(noise_tex2,tc*0.2).b,64.0))*100.0 +
            mix(smoothstep(-2.0,1.0,obj_pos.z) * vec3(0.3,0.3,0.1)*(0.5+0.5*dot(n,normalize(vec3(1.0)))),vec3(1.0,1.0,0.9),max(texture(cubemap_tex,r).r,texture(cubemap_tex,-r).r))+vec3(0.5+0.5*-r.y)*0.3;
    colour.a=pow(max(0.0,dot(colour.rgb,vec3(1.0/3.0)))*0.2,2.0);
    //colour.rgb=vec3(0.2,0.23,0.4)*mix(0.9,2.0,texture(noise_tex,tc).r)+vec3(pow(abs(1.0-dot(v,norm)),2.0))+vec3(pow(max(0.0,dot(v,norm)),8.0)*0.2);
}

