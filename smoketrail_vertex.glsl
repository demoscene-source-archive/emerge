#version 330

layout(location = 0) in vec2 vertex;
uniform float time;

const float pi=3.1415926;

void main()
{
    vec2 p=vertex;
    p.y*=1.5;
    vec4 v;
    float t=smoothstep(0.0,3.0,max(0.0,time-p.y))*3.0;
    float r=0.04*smoothstep(0.0,0.2,t)*mix(0.5,2.0,abs(cos(p.y*12.0)))*(1.0-smoothstep(1.0,3.0,t))*(1.0-smoothstep(1.4,1.5,p.y))+
            pow(t/(t+2.0),2.0)*mix(0.8,1.0,smoothstep(0.0,0.1,p.y))*mix(1.0,0.0,smoothstep(0.5,1.0,p.y));
    v.x=cos(p.x*pi*2.0)*r;
    v.y=(p.y-t/(t+1.0)*p.y*mix(1.0,0.2,pow(p.y/1.5,64.0))+p.y*cos(p.x*10.0+time)*0.01)*p.y+t*p.y*0.05-time*0.05;
    v.z=sin(p.x*pi*2.0)*r;
    v.z=-v.z;
    v.y-=0.15;
    v.xyz=v.xzy*300.0;
    v.w=1.0;


/*
    vec4 v;
    v.x=vertex.x*100.0;
    v.y=vertex.y*100.0;
    v.z=time*0.000001;
    v.w=1.0;
*/

    gl_Position = v;
}
