#version 330

uniform sampler2D fb_final;
uniform sampler2D noise_tex;

layout(location = 0) out vec4 colour;

noperspective in vec2 coord;

void main()
{
    colour.a = 1.0;
    vec4 s = texelFetch(fb_final, ivec2(gl_FragCoord.xy),0);
    colour.rgb = sqrt(clamp(s.rgb,vec3(0.0),vec3(1.0)));
    colour.rgb *= 0.5 + 0.5*pow( 16.0*coord.x*coord.y*(1.0-coord.x)*(1.0-coord.y), 0.25 );
    colour.rgb += texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy,textureSize(noise_tex, 0))), 0).rgb / 128.0;    
    colour.rgb+=vec3(s.a);
}

