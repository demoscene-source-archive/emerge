#version 330

uniform sampler2D fb_normal;
uniform sampler2D fb_position;
uniform sampler2D material_palette;
uniform sampler2D fb_velocity;
uniform sampler2D crystal_tex;
uniform sampler2D noise_tex;
uniform sampler2D stars_tex;
uniform samplerCube skybox_tex;
uniform mat4 view_to_world;

uniform vec3 viewspace_sun_direction;
uniform vec3 viewspace_up;
uniform vec3 cave_point;

uniform float white_background;
uniform float darkness;
uniform float fade;

uniform float stars;

layout(location = 0) out vec4 colour;

noperspective in vec2 coord;
noperspective in vec3 ray_direction;

// www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
// http://www.simonwallner.at/projects/atmospheric-scattering
        
// constants for atmospheric scattering
// SI units unless otherwise noted.

const float e = 2.71828182845904523536028747135266249775724709369995957f;
const float pi = 3.141592653589793238462643383279502884197169;

const float n = 1.0003; // refractive index of air
const float N = 2.545E25; // number of molecules per unit volume for air at
						// 288.15K and 1013mb (sea level -45 celsius)
const float pn = 0.035;	// depolatization factor for standard air

// wavelength of used primaries, according to preetham
const vec3 lambda = vec3(680E-9, 550E-9, 450E-9);

// mie stuff
// K coefficient for the primaries
const vec3 K = vec3(0.686f, 0.678f, 0.666f);
const float v = 4.0f;


// optical length at zenith for molecules
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;

const float E = 1000.0f;
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324f;

// earth shadow hack
const float cutoffAngle = pi/2.0f;
const float steepness = 0.5f;





/**
 * Compute total rayleigh coefficient for a set of wavelengths (usually
 * the tree primaries)
 * @param lambda wavelength in m
 */
vec3 totalRayleigh(vec3 lambda)
{
	return (8 * pow(pi, 3) * pow(pow(n, 2) - 1, 2) * (6 + 3 * pn)) / (3 * N * pow(lambda, vec3(4)) * (6 - 7 * pn));
}

/** Reileight phase function as a function of cos(theta)
 */
float rayleighPhase(float cosTheta)
{
	/**
	 * NOTE: There are a few scale factors for the phase funtion
	 * (1) as given bei Preetham, normalized over the sphere with 4pi sr
	 * (2) normalized to integral = 1
	 * (3) nasa: integrates to 9pi / 4, looks best
	 */
	 
//	return (3.0f / (16.0f*pi)) * (1.0f + pow(cosTheta, 2));
//	return (1.0f / (3.0f*pi)) * (1.0f + pow(cosTheta, 2));
	return (3.0f / 4.0f) * (1.0f + pow(cosTheta, 2));
}

/**
 * total mie scattering coefficient
 * @param lambda set of wavelengths in m
 * @param K corresponding scattering param
 * @param T turbidity, somewhere in the range of 0 to 20

 */
vec3 totalMie(vec3 lambda, vec3 K, float T)
{
	// not the formula given py Preetham.
	float c = (0.2f * T ) * 10E-18;
	return 0.434 * c * pi * pow((2 * pi) / lambda, vec3(v - 2)) * K;
}


/**
 * Henyey-Greenstein approximation as a function of cos(theta)
 * @param cosTheta 
 * @param g goemetric constant that defines the shape of the ellipse.
 */
float hgPhase(float cosTheta, float g)
{
	return (1.0f / (4.0f*pi)) * ((1.0f - pow(g, 2)) / pow(1.0f - 2.0f*g*cosTheta + pow(g, 2), 1.5));
}

float sunIntensity(float zenithAngleCos)
{
//	return E;
	return E * max(0.0f, 1.0f - exp(-((cutoffAngle - acos(zenithAngleCos))/steepness)));
}


uniform float reileighCoefficient = 0.05f;
uniform float mieCoefficient = 0.053f;
uniform float mieDirectionalG = 0.75f;
uniform float turbidity = 1.4f;
const vec2 samples[16] = vec2[16](
 vec2(0.000000, -0.333333),
 vec2(-0.500000, 0.333333),
 vec2(0.500000, -0.777778),
 vec2(-0.750000, -0.111111),
 vec2(0.250000, 0.555556),
 vec2(-0.250000, -0.555556),
 vec2(0.750000, 0.111111),
 vec2(0.125000, -0.925926),
 vec2(-0.375000, -0.259259),
 vec2(0.625000, 0.407407),
 vec2(-0.625000, -0.703704),
 vec2(0.375000, -0.037037),
 vec2(-0.125000, 0.629630),
 vec2(0.875000, -0.481481),
 vec2(-0.937500, 0.185185),
 vec2(0.062500, 0.851852)
);

float AO(vec3 n, vec3 c, vec2 p)
{
    float sh = 0.0;
    float scale = 0.2 / length(c);
    float rd = dot(n, c);
    for(int i = 0; i < samples.length(); i += 1)
    {
        vec2 ss = reflect(samples[i], texelFetch(noise_tex, ivec2(mod(gl_FragCoord.xy, textureSize(noise_tex, 0).xy)), 0).rg);
        vec3 s = texture(fb_position, p + vec2(ss * scale)).rgb;
        float d = dot(s, n) - rd;
        sh += smoothstep(0.01, 0.1, d) - smoothstep(0.2, 0.3, d);
    }
    return 1.0 - sh / float(samples.length());
}

void main()
{
        vec4 pb = texture(fb_position, coord);
    vec4 nc = texture(fb_normal, coord);

  if(nc.w < 1./1024.)
   pb.xyz = ray_direction * 1000.0;

        
        // www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
        // http://www.simonwallner.at/projects/atmospheric-scattering
        
        // extinction (absorbtion + out scattering)
        // rayleigh coefficients
        vec3 betaR = totalRayleigh(lambda) * reileighCoefficient;

        // mie coefficients
        vec3 betaM = totalMie(lambda, K, turbidity) * mieCoefficient;
       
        // in scattering
        float cosTheta = max(0.0,dot(normalize(pb.xyz), viewspace_sun_direction));

        float rPhase = rayleighPhase(cosTheta);
        vec3 betaRTheta = betaR * rPhase;

        float mPhase = hgPhase(cosTheta, mieDirectionalG);
        vec3 betaMTheta = betaM * mPhase;

        float sun_intensity = sunIntensity(dot(viewspace_sun_direction, viewspace_up));
        vec3 Lin = sun_intensity * (betaRTheta + betaMTheta) / (betaR + betaM);


    
            vec3 Fex = exp(-((betaR + betaM) * length(pb.xyz)));



    colour.a = 0.0;


    if(nc.w >= 1./1024.)
    {
        vec3 n = normalize((nc.rgb - vec3(0.5)) * 2.0);   
        
        vec3 world = (view_to_world * vec4(pb.xyz, 1.0)).xyz;
        float cave_shadow = smoothstep(5.0,15.0,distance(world,cave_point));
        
        vec3 diffuse = min(vec3(1.0),mix(vec3(0.5), texture(material_palette, vec2(0.0, nc.w)).rgb, 0.8)*4.0);
        vec4 flags = texture(material_palette, vec2(1.5/4.0, nc.w));

        //diffuse=mix(diffuse,min(vec3(1.0),diffuse*3.0),flags.b);
        diffuse=mix(diffuse,diffuse*3.0,flags.b);
        
        float ao = AO(n, pb.xyz, coord);

        cave_shadow = max(cave_shadow, 1.0 - flags.r);
        
        float voxel_shadow = 1.0-smoothstep(0.0,0.1,texture(fb_velocity, coord).z) * 0.5;//max(0.7,pow(texture(fb_velocity, coord).z, 2.0));
        
        ao *= voxel_shadow;
        
        vec3 scol=mix(vec3(1.0, 0.5, 0.5),vec3(1.0), pow(clamp(dot(viewspace_sun_direction, viewspace_up),0.0,1.0),1.0/2.0));
        ao *= sun_intensity*2.2e-3;
        vec3 sunlight_colour = vec3(sun_intensity)*1.6e-3*scol*vec3(1.1,1.1,0.9);
        
        //colour.rgb = vec3(ao);// * (0.5 + 0.5 * dot(n, normalize(vec3(1.0))));

        colour.rgb = cave_shadow * (ao * (scol * vec3(0.15,0.15,0.25) * (0.5 + 0.5 * n.y) * 1.2 + voxel_shadow * pb.w * sunlight_colour * max(0.0, dot(n, normalize(viewspace_sun_direction))))) * diffuse;
        colour.rgb += voxel_shadow * 0.3 * mix(diffuse, vec3(1.0), 0.25) * pow(max(0.0,1.0-dot(normalize(-pb.xyz),n)),4.0) * pow(max(0.0, sun_intensity*2e-3),2.0) * mix(0.04, 1.0, cave_shadow);

        vec3 world_ray = mat3(view_to_world) * ray_direction;
        colour.rgb = colour.rgb * Fex + (Lin + vec3(80.0 * (1.0 - cave_shadow) * mix(0.6,1.0,texture(noise_tex, vec2(coord.x+atan(world_ray.z, world_ray.x)+view_to_world[3].z*-0.15,coord.y+view_to_world[3].y*0.25) ).b))) * (vec3(1.0) - Fex);
            //colour.rgb=vec3(pb.w);

        //colour.rgb = vec3(max(0.0, dot(n, normalize(vec3(1.0)))));

        if(nc.w==1.0)
        {
            vec3 ha = normalize(normalize(pb.xyz)+normalize(viewspace_sun_direction));
            colour.rgb = mix(vec3(0.05,0.05,0.1), vec3(1.0,1.0,0.2),  (0.5 + 0.5 * dot(n, normalize(viewspace_sun_direction)))) * 0.9 + 1.6 * vec3(pow(max(0.0,dot(n,ha)),4.0));
            //colour.a = pow(max(0.0, dot(colour.rgb, vec3(1.0 / 3.0))), 2.0);
            colour.a = pow(max(0.0,dot(n,ha)),4.0);
        }

        colour.rgb += texture(crystal_tex, coord).rgb;    
        vec3 r = normalize(pb.xyz) - n * dot(normalize(pb.xyz), n) * 2.0;
        colour.rgb += (1.0 - cave_shadow) * texture(crystal_tex, (coord-vec2(0.5)) + vec2(0.5) + r.xy* 0.1*vec2(1,1) ).rgb * 0.12 * smoothstep(0.0,0.2,1.0-dot(normalize(pb.xyz), r)); 
        colour.rgb+=flags.b*vec3(pow(max(0.0,dot(n,normalize(normalize(viewspace_sun_direction)+normalize(-pb.xyz)))),32.0));
        colour.rgb=mix(colour.rgb,vec3(0.0),darkness);
  
        //colour.rgb*=4.0;
        //colour.rgb=diffuse;
    }
    else
    {
        colour.rgb = colour.rgb * Fex + Lin * (vec3(1.0) - Fex);
        vec3 sv = normalize(mat3(view_to_world) * -ray_direction);
        colour.rgb += vec3(texture(skybox_tex, sv ).r * 0.8);
        
        vec2 stars_uv=sv.yz;
        vec3 abs_sv=abs(sv);
        if(abs_sv.z>abs_sv.x && abs_sv.z>abs_sv.y)
            stars_uv=sv.xy;
        else if(abs_sv.y>abs_sv.x && abs_sv.y>abs_sv.z)
            stars_uv=sv.xz;
        
        //vec3 s = vec3(texture(stars_tex, vec2(atan(sv.z, sv.x),sv.y)).r);
        vec3 s = vec3(texture(stars_tex, stars_uv).r);
        float sa = clamp(coord.y * stars, 0.0, 1.0);
        colour.rgb = mix(colour.rgb, s * 1.4, sa);
        colour.a = pow(clamp(sa * s.r, 0.0, 1.0), 2.0) * 10.0;
        if(white_background > 0.5)
            colour = vec4(1.0,1.0,1.0,0.0);
    }
 
    colour.rgb *= fade;
}

