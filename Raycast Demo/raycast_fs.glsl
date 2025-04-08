#version 430
layout(binding = 0) uniform sampler2D backface_tex; 
layout(location = 1) uniform float time;
layout(location = 2) uniform int pass;
layout(location = 3) uniform int mode = 1;
layout(location = 4) uniform vec4 slider = vec4(1.0);

layout(std140, binding = 0) uniform SceneUniforms
{
	mat4 PV;	//camera projection * view matrix
	mat4 PVinv;
	vec4 eye_w;	//world-space eye position
};

layout(std140, binding = 1) uniform LightUniforms
{
   vec4 La;	//ambient light color
   vec4 Ld;	//diffuse light color
   vec4 Ls;	//specular light color
   vec4 light_w; //world-space light position
};

layout(std140, binding = 2) uniform MaterialUniforms
{
   vec4 ka;	//ambient material color
   vec4 kd;	//diffuse material color
   vec4 ks;	//specular material color
   float shininess; //specular exponent
};

in VertexData
{
   vec3 pw;       //world-space vertex position
} inData;   //block is named 'inData'

out vec4 fragcolor; //the output color for this fragment    

/////////////////////////////////////////////////////////
//Function forward declarations
// 
//the raymarching loop
vec4 raycast_sdf_scene(vec3 rayStart, vec3 rayStop);

//determines the color when ray misses the scene
vec4 sky_color(vec3 dir);

//Scene representation as a signed distance function
float dist_to_scene(vec3 pos);

//Computes the normal vector as gradient of scene sdf
vec3 normal(vec3 pos);

//Computes Phong lighting
vec4 lighting(vec3 pos, vec3 rayDir);


//shape function declarations
float sdSphere( vec3 p, float s );
float sdBox( vec3 p, vec3 b );
float sdOctahedron( vec3 p, float s);
float sdRoundedCylinder( vec3 p, float ra, float rb, float h );
float sdCapsule( vec3 p, vec3 a, vec3 b, float r );

//shape modifiers
float opUnion( float d1, float d2 ) { return min(d1,d2); }
float opIntersection( float d1, float d2 )
{
    return max(d1,d2);
}
float opSubtraction( float d1, float d2 )
{
    return max(-d1,d2);
}
float opSmoothUnion( float d1, float d2, float k )
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

// For more distance functions see
// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

// Soft shadows
// http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm

// WebGL example and a simple ambient occlusion approximation
// https://www.shadertoy.com/view/Xds3zN


void main(void)
{   
   if(pass == 0)
	{
		fragcolor = vec4(inData.pw, 1.0); //write cube back face positions to texture
	}
   else if(pass == 1)
   {
		//DEBUG: uncomment to see front faces
		//fragcolor = vec4(inData.pw, 1.0); //draw cube front faces
		//return;

      vec3 rayStart = inData.pw;
		vec3 rayStop = texelFetch(backface_tex, ivec2(gl_FragCoord.xy), 0).xyz;
      fragcolor = raycast_sdf_scene(rayStart, rayStop);

		//gamma
		fragcolor = pow(fragcolor, vec4(0.45, 0.45, 0.45, 1.0));
   }
}

//You shouldn't need to change this function for Lab 3
vec4 raycast_sdf_scene(vec3 rayStart, vec3 rayStop)
{
	const int MaxSamples = 10000; //max number of steps along ray

	vec3 rayDir = normalize(rayStop-rayStart);	//ray direction unit vector
	float travel = distance(rayStop, rayStart);	
	float stepSize = travel/MaxSamples;	//initial raymarch step size
	vec3 pos = rayStart;				      //position along the ray
	vec3 step = rayDir*stepSize;		   //displacement vector along ray
	
	for (int i=0; i < MaxSamples && travel > 0.0; ++i, pos += step, travel -= stepSize)
	{
		float dist = dist_to_scene(pos); //How far are we from the shape we are raycasting?

		//Distance tells us how far we can safely step along ray without intersecting surface
		stepSize = dist;

		step = rayDir*stepSize;
		
		//Check distance, and if we are close then perform lighting
		const float eps = 1e-6;
		if(dist <= eps)
		{	
			pos += step;
			return lighting(pos, rayDir);
		}	
	}
	//If the ray never intersects the scene then output clear color
	return sky_color(rayDir);
}

//This function defines the scene
float dist_to_scene(vec3 pos)
{
	vec3 Cpos = pos-vec3(0.0,-0.2*slider.y,0.0);
	vec3 rotatedP = vec3(Cpos.x * cos(time) - Cpos.z * sin(time),Cpos.y,Cpos.x * sin(time) + Cpos.z * cos(time));
	float d1 = sdOctahedron( rotatedP, 0.3);

	vec3 Cylinder_cen = vec3(0.0,0.6*slider.x,0.0);
	float d2 = sdRoundedCylinder( pos-Cylinder_cen, 0.6, 0.1, 0.1*slider.w );
	vec3 CapsuleA=vec3(0.0,0.8,0.0);
	vec3 CapsuleB=vec3(0.8*sin(time), 0.0, 0.8*cos(time));
	float d3 = sdCapsule( pos, CapsuleA, CapsuleB, 0.1 );

	vec3 Cylinder_cen2 = vec3(0.0,-0.6*slider.z,0.0);
	float d4 = sdRoundedCylinder( pos-Cylinder_cen2, 0.6, 0.1, 0.1);

	return opSmoothUnion(opUnion(opIntersection(d3, d2),d1),d4,0.1);
	//return opUnion(d2, d3);
}


float shadow( in vec3 ro, in vec3 rd, float mint, float maxt )
{
    float t = mint;
    for( int i=0; i<256 && t<maxt; i++ )
    {
        float h = dist_to_scene(ro + rd*t);
        if( h<0.001 )
            return 0.0;
        t += h;
    }
    return 1.0;
}

//compute lighting on the intersected surface
vec4 lighting(vec3 pos, vec3 rayDir)
{
   vec4 ambient_term = ka*La;

   vec3 nw = normal(pos);			//world-space unit normal vector
   vec3 lw = normalize(light_w.xyz - pos);	//world-space unit light vector
   vec4 diffuse_term = kd*Ld*max(0.0, dot(nw, lw));

   vec3 vw = -rayDir;	//world-space unit view vector
   vec3 rw = reflect(-lw, nw);	//world-space unit reflection vector

   vec4 specular_term = ks*Ls*pow(max(0.0, dot(rw, vw)), shininess);
   if(shadow( pos, lw, 0.1, 100.0 )==0.0){return ambient_term;}
   return ambient_term + diffuse_term + specular_term;
}

//normal vector of the shape we are drawing.
//DO not change this function for Lab 3
vec3 normal(vec3 pos)
{
	const float h = 0.001;
	const vec3 Xh = vec3(h, 0.0, 0.0);	
	const vec3 Yh = vec3(0.0, h, 0.0);	
	const vec3 Zh = vec3(0.0, 0.0, h);	

	//compute gradient using central differences
	return normalize(vec3(dist_to_scene(pos+Xh)-dist_to_scene(pos-Xh), dist_to_scene(pos+Yh)-dist_to_scene(pos-Yh), dist_to_scene(pos+Zh)-dist_to_scene(pos-Zh)));
}

vec4 sky_color(vec3 dir)
{
   return mix(vec4(0.99, 0.8, 0.89,1.0),vec4(0.8, 0.98, 0.99,1.0),5*dir.y+0.3);
}

float sdSphere( vec3 p, float s )
{
	return length(p)-s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdOctahedron( vec3 p, float s)
{
  p = abs(p);
  return (p.x+p.y+p.z-s)*0.57735027;
}

float sdRoundedCylinder( vec3 p, float ra, float rb, float h )
{
  vec2 d = vec2( length(p.xz)-2.0*ra+rb, abs(p.y) - h );
  return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rb;
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}
