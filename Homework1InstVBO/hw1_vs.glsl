#version 430            
layout(location = 0) uniform mat4 M;
layout(location = 1) uniform float time;

layout(std140, binding = 0) uniform SceneUniforms
{
   mat4 PV;	//camera projection * view matrix
   vec4 eye_w;	//world-space eye position
};

layout(location = 0) in vec3 pos_attrib; //this variable holds the position of mesh vertices
layout(location = 1) in vec2 tex_coord_attrib;
layout(location = 2) in vec3 normal_attrib;  
layout (location = 3) in vec4 color;
layout (location = 4) in mat4 model_matrix;

out vec4 instcolor;

out VertexData
{
   vec2 tex_coord;
   vec3 pw;       //world-space vertex position
   vec3 nw;   //world-space normal vector
} outData; 

void main(void)
{
	gl_Position = PV*M*model_matrix*vec4(pos_attrib, 1.0); //transform vertices and send result into pipeline
	//gl_Position = PV*M*vec4(pos_attrib.x+3*color.x,pos_attrib.y+3*color.y,pos_attrib.z+3*color.z, 1.0);
	//Use dot notation to access members of the interface block
	outData.tex_coord = tex_coord_attrib;           //send tex_coord to fragment shader
	outData.pw = vec3(M*vec4(pos_attrib, 1.0));		//world-space vertex position
	outData.nw = vec3(M*vec4(normal_attrib, 0.0));	//world-space normal vector

	instcolor=color;
}