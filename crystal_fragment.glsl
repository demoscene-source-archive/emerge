#version 330

in vec3 pos;
in vec3 norm;
in vec2 tc;

uniform sampler2D noise_tex;
uniform vec3 in_colour;

layout(location = 0) out vec4 colour;

void main()
{
    vec3 v=normalize(-pos);
    colour.rgb=in_colour*mix(0.9,2.0,texture(noise_tex,tc).r)+vec3(pow(abs(1.0-dot(v,norm)),2.0))+vec3(pow(max(0.0,dot(v,norm)),8.0)*0.2);
}

