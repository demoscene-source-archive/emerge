#version 330

uniform sampler2D fb_normal;
uniform sampler2D fb_position;
uniform sampler2D noise_tex;
uniform vec3 in_colour;

flat in vec3 origin;
in vec3 coord;

layout(location = 0) out vec4 colour;

void main()
{
    vec2 coord2 = (coord.xy / coord.z + vec2(1.0)) * 0.5;
    
    vec4 pb = texture(fb_position, coord2);
    vec4 nc = texture(fb_normal, coord2);

    vec3 n = normalize((nc.rgb - vec3(0.5)) * 2.0); 

    colour.a = 1.0;
 
    vec3 l = normalize(origin - pb.xyz);
 
    colour.rgb = in_colour * pow(max(0.0,dot(n,l)),2.0) * pow(max(0.0,1.0-distance(pb.xyz, origin)), 2.0) * 0.7;
}

