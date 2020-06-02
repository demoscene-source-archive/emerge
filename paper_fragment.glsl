#version 330

noperspective in vec2 coord;
layout(location = 0) out vec2 colour;


float hash(float n)
{
   n=mod(n,64.0);
    return fract(sin(n)*43758.5453);
}

float noise(vec2 p)
{
   return hash(p.x + p.y*57.0);
}

float smoothNoise2(vec2 p)
{
   vec2 p0 = floor(p + vec2(0.0, 0.0));
   vec2 p1 = floor(p + vec2(1.0, 0.0));
   vec2 p2 = floor(p + vec2(0.0, 1.0));
   vec2 p3 = floor(p + vec2(1.0, 1.0));
   vec2 pf = fract(p);
   return mix( mix(noise(p0), noise(p1), pf.x), 
               mix(noise(p2), noise(p3), pf.x), pf.y);
}

vec2 cellPoint(vec2 cell)
{
   return vec2(noise(cell)+cos(cell.y)*0.3, noise(cell*0.3)+sin(cell.x)*0.3);
}

vec3 voronoi2(vec2 t,float pw)
{
   vec2 p = floor(t);
   vec3 nn=vec3(1e10);

   float wsum=0.0;
   vec3 cl=vec3(0.0);
   for(int y = -1; y < 2; y += 1)
      for(int x = -1; x < 2; x += 1)
      {
         vec2 b = vec2(float(x), float(y));
         vec2 q = b + p;
         vec2 q2 = q-floor(q/8.0)*8.0;
         vec2 c = q + cellPoint(q2);
         vec2 r = c - t;
         vec2 r2=r;

         float d = dot(r, r);
         float w=pow(smoothstep(0.0,1.0,1.0-abs(r2.x)),pw)*pow(smoothstep(0.0,1.0,1.0-abs(r2.y)),pw);

         cl+=vec3(0.5+0.5*cos((q2.x+q2.y*119.0)*8.0))*w;
         wsum+=w;

         nn=mix(vec3(q2,d),nn,step(nn.z,d));
      }

   return pow(cl/wsum,vec3(0.5))*2.0;
}

vec3 voronoi(vec2 t)
{
   return voronoi2(t*0.25,16.0)*(0.0+1.0*voronoi2(t*0.5+vec2(voronoi2(t*0.25,16.0)),2.0))+voronoi2(t*0.5,4.0)*0.5;
}

void main()
{
   colour.rg = vec2(0.0);
   
   for(int y=0;y<2;y+=1)
      for(int z=0;z<2;z+=1)
      {
         vec2 tt = coord * 32.0+vec2(y,z)*1.0/1024.0;
      
         float x=voronoi(tt).r;
         float x1=voronoi(tt+vec2(1e-2,0.0)).r;
         float x2=voronoi(tt+vec2(0.0,1e-2)).r;

         colour.rg+=vec2((x1-x)/1e-2,(x2-x)/1e-2)*0.1;
      }
   colour.rg/=4.0;
   colour.rg=(colour.rg+vec2(1.0))*0.5;
}


