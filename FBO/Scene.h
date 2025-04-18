#pragma once

#include <glm/glm.hpp>

namespace Scene
{
   void DrawGui(GLFWwindow* window);
   void Display(GLFWwindow* window);
   void Idle();
   void Init();
   void ReloadShader();
   void UpdateFbo();
   int readPickID(int x, int y);
   void moveInstance(float x, float y);
   void resetPickID();
   extern const int InitWindowWidth;
   extern const int InitWindowHeight;
   extern int WindowWidth;
   extern int WindowHeight;

   namespace Camera
   {
      extern glm::mat4 P;
      extern glm::mat4 V;

      extern float Aspect;
      extern float NearZ;
      extern float FarZ;
      extern float Fov;

      void UpdateP();
   }
};
