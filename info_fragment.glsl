#version 330

uniform sampler2D tex;
uniform float alpha;

noperspective in vec2 coord;

layout(location = 0) out vec4 colour;

void main()
{
   colour=vec4(0.0);
   colour.a=(1.0-texture(tex,coord).r)*alpha;
}
