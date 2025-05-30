#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Uniforms
{
   void Init();
   void BufferSceneData();
   void BufferLightData();
   void BufferMaterialData();

   //This structure mirrors the uniform block declared in the shader
   struct SceneUniforms
   {
      glm::mat4 PV;	//camera projection * view matrix
      glm::vec4 eye_w = glm::vec4(0.0f, 0.0f, 3.0f, 1.0f);	//world-space eye position
   };

   struct LightUniforms
   {
      glm::vec4 La = glm::vec4(0.65f, 0.7f, 0.7f, 1.0f);	//ambient light color
      glm::vec4 Ld = glm::vec4(0.75f, 0.7f, 0.65f, 1.0f);	//diffuse light color
      glm::vec4 Ls = glm::vec4(0.85f, 0.8f, 0.75f, 1.0f);	//specular light color
      glm::vec4 light_w = glm::vec4(0.0f, 1.2, 1.0f, 1.0f); //world-space light position
   };

   struct MaterialUniforms
   {
      glm::vec4 ka = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);	//ambient material color
      glm::vec4 kd = glm::vec4(0.4f, 0.3f, 0.2f, 1.0f);	//diffuse material color
      glm::vec4 ks = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	//specular material color
      glm::vec4 F0 = glm::vec4(0.9f, 0.8f, 0.4f, 1.0f);
      float m = 0.25f;         //specular exponent
   };

   extern SceneUniforms SceneData;
   extern LightUniforms LightData;
   extern MaterialUniforms MaterialData;

   //IDs for the buffer objects holding the uniform block data
   extern GLuint scene_ubo;
   extern GLuint light_ubo;
   extern GLuint material_ubo;

   namespace UboBinding
   {
      //These values come from the binding value specified in the shader block layout
      extern int scene;
      extern int light;
      extern int material;
   };

   //Locations for the uniforms which are not in uniform blocks
   namespace UniformLocs
   {
      extern int M; //model matrix
      extern int time;
      extern int mode;
   };
};
