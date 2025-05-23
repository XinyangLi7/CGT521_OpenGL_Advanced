#version 430            
layout(location = 0) uniform mat4 M;
layout(location = 1) uniform float time;
layout(location = 2) uniform int pass = 0;
layout(location = 4) uniform int pickedID;


layout(std140, binding = 0) uniform SceneUniforms
{
   mat4 PV;	//camera projection * view matrix
   vec4 eye_w;	//world-space eye position
};

layout(location = 0) in vec3 pos_attrib; //this variable holds the position of mesh vertices
layout(location = 1) in vec2 tex_coord_attrib;
layout(location = 2) in vec3 normal_attrib;  
layout (location = 3) in mat4 model_matrix;

out VertexData
{
   vec2 tex_coord;
   vec3 pw;       //world-space vertex position
   vec3 nw;   //world-space normal vector
} outData; 

flat out int InstanceID;

const vec4 quad[4] = vec4[] (vec4(-1.0, 1.0, 0.0, 1.0), 
							vec4(-1.0, -1.0, 0.0, 1.0), 
							vec4( 1.0, 1.0, 0.0, 1.0), 
							vec4( 1.0, -1.0, 0.0, 1.0) );

void main(void)
{
	if(pass==0)
	{
	InstanceID = gl_InstanceID+1;
	vec3 offset=vec3(gl_InstanceID%3-1,0.0,gl_InstanceID/3-1);
	if(pickedID!=InstanceID)
	{
	offset.z+=(pos_attrib.x+0.2)*0.1*sin(4*pos_attrib.x+7*time+gl_InstanceID*3);
	}
	gl_Position = model_matrix*PV*M*vec4(pos_attrib+0.5*offset, 1.0); //transform vertices and send result into pipeline
	
	//Use dot notation to access members of the interface block
	outData.tex_coord = tex_coord_attrib;           //send tex_coord to fragment shader
	outData.pw = vec3(M*vec4(pos_attrib, 1.0));		//world-space vertex position
	outData.nw = vec3(M*vec4(normal_attrib, 0.0));	//world-space normal vector
	
	}
	if(pass==1) //full screen quad
   {
      gl_Position = quad[ gl_VertexID ]; //get clip space coords out of quad array
      outData.tex_coord = 0.5*(quad[ gl_VertexID ].xy + vec2(1.0)); 
   }
}