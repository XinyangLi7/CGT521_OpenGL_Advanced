#version 430
layout(binding = 0) uniform sampler2D diffuse_tex; 
layout(location = 1) uniform float time;
layout(location = 2) uniform int mode;

layout(std140, binding = 0) uniform SceneUniforms
{
   mat4 PV;	//camera projection * view matrix
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
   vec4 F0;
   float m; //specular exponent
};

in VertexData
{
   vec2 tex_coord;
   vec3 pw;       //world-space vertex position
   vec3 nw;   //world-space normal vector
} inData;   //block is named 'inData'

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
   //Compute per-fragment Phong lighting
   //vec4 ktex = texture(diffuse_tex, inData.tex_coord);
	
   vec4 ambient_term = ka*La;

   const float eps = 1e-8; //small value to avoid division by 0
   float d = distance(light_w.xyz, inData.pw.xyz);
   //float atten = 1.0/(d*d+eps); //d-squared attenuation

   vec3 nw = normalize(inData.nw);			//world-space unit normal vector
   vec3 lw = normalize(light_w.xyz - inData.pw.xyz);	//world-space unit light vector
   vec4 diffuse_term = kd*Ld*max(0.0, dot(nw, lw));

   vec3 vw = normalize(eye_w.xyz - inData.pw.xyz);	//world-space unit view vector
   vec3 rw = reflect(-lw, nw);	//world-space unit reflection vector
   vec3 hw=normalize(lw+vw);

   vec4 specular_term = ks*Ls/3.14/max(eps,dot(nw,vw));
   if(mode==0||mode==1){
      vec4 F = F0+(1-F0)*pow(1-max(0.0,dot(nw,vw)),5);
	  specular_term *= F;
   }

   if(mode==0||mode==2){
      float cos2a=pow(max(0.0,dot(nw,hw)),2);
      float D = exp((cos2a-1)/(cos2a*m*m+eps))/(4*pow(m*cos2a,2)+eps);
	  specular_term *= D;
   }

   if(mode==0||mode==3){
      float Gpara=2*max(0.0,dot(nw,hw))/max(eps,dot(vw,hw));
	  float G=min(min(1,Gpara*max(0.0,dot(nw,vw))),Gpara*max(0.0,dot(nw,lw)));
	  specular_term *= G;
   }

   fragcolor = ambient_term + diffuse_term + specular_term;
}

