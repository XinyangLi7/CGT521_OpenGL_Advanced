//When using this as a template, be sure to make these changes in the new project: 
//1. In Debugging properties set the Environment to PATH=%PATH%;$(SolutionDir)\lib;
//2. Copy assets (mesh and texture) to new project directory

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Scene.h"
#include "Uniforms.h"
#include "InitShader.h"    //Functions for loading shaders from text files
#include "VideoRecorder.h"      //Functions for saving videos
#include "DebugCallback.h"
#include "Surf.h"

#define NUM_INSTANCES 9

const int Scene::InitWindowWidth = 1024;
const int Scene::InitWindowHeight = 1024;

static const std::string vertex_shader("hw1_vs.glsl");
static const std::string fragment_shader("hw1_fs.glsl");
GLuint shader_program = -1;

surf_vao surface;
//surf_vao surf[6];
//int draw_surf = 0; // Which of the previous VAOs to draw

float angle = 0.0f;
float scale = 0.03f;
bool recording = false;

namespace Scene
{
   namespace Camera
   {
      glm::mat4 P;
      glm::mat4 V;

      float Aspect = 1.0f;
      float NearZ = 0.1f;
      float FarZ = 100.0f;
      float Fov = glm::pi<float>() / 4.0f;

      void UpdateP()
      {
         P = glm::perspective(Fov, Aspect, NearZ, FarZ);
      }
   }
}


// This function gets called every time the scene gets redisplayed
void Scene::Display(GLFWwindow* window)
{
   //Clear the screen to the color previously specified in the glClearColor(...) call.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   Camera::V = glm::lookAt(glm::vec3(Uniforms::SceneData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   Uniforms::SceneData.PV = Camera::P * Camera::V;
   Uniforms::BufferSceneData();

   glUseProgram(shader_program);
   
   //Set uniforms
   glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(scale));
   glUniformMatrix4fv(Uniforms::UniformLocs::M, 1, false, glm::value_ptr(M));

   //Draw a surface from currently selected VAO
   glBindVertexArray(surface.vao);
   glDrawElementsInstanced(GL_TRIANGLE_STRIP, surface.num_indices, GL_UNSIGNED_INT, 0, 9);

   DrawGui(window);

   if (recording == true)
   {
      glFinish();
      glReadBuffer(GL_BACK);
      int w, h;
      glfwGetFramebufferSize(window, &w, &h);
      VideoRecorder::EncodeBuffer(GL_BACK);
   }

   /* Swap front and back buffers */
   glfwSwapBuffers(window);
}

void Scene::DrawGui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   //Draw Gui
   ImGui::Begin("Debug window");
   if (ImGui::Button("Quit"))
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }

   const int filename_len = 256;
   static char video_filename[filename_len] = "capture.mp4";
   static bool show_imgui_demo = false;

   if (recording == false)
   {
      if (ImGui::Button("Start Recording"))
      {
         int w, h;
         glfwGetFramebufferSize(window, &w, &h);
         recording = true;
         const int fps = 60;
         const int bitrate = 4000000;
         VideoRecorder::Start(video_filename, w, h, fps, bitrate); //Uses ffmpeg
      }
   }
   else
   {
      if (ImGui::Button("Stop Recording"))
      {
         recording = false;
         VideoRecorder::Stop(); //Uses ffmpeg
      }
   }
   ImGui::SameLine();
   ImGui::InputText("Video filename", video_filename, filename_len);


   ImGui::SliderFloat("View angle", &angle, -glm::pi<float>(), +glm::pi<float>());
   ImGui::SliderFloat("Scale", &scale, -1.0f, +1.0f);

   static bool enable_culling = false;
   if (ImGui::Checkbox("Backface culling", &enable_culling))
   {
      if (enable_culling) glEnable(GL_CULL_FACE);
      else glDisable(GL_CULL_FACE);
   }

   static int polygon_mode = GL_FILL;
   ImGui::Text("PolygonMode ="); ImGui::SameLine();
   ImGui::RadioButton("GL_FILL", &polygon_mode, GL_FILL); ImGui::SameLine();
   ImGui::RadioButton("GL_LINE", &polygon_mode, GL_LINE); ImGui::SameLine();
   ImGui::RadioButton("GL_POINT", &polygon_mode, GL_POINT);
   glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

   /*if (ImGui::Button("Show ImGui Demo Window"))
   {
      show_imgui_demo = true;
   }*/
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

   ImGui::End();

   if (show_imgui_demo == true)
   {
      ImGui::ShowDemoWindow(&show_imgui_demo);
   }

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::Idle()
{
   float time_sec = static_cast<float>(glfwGetTime());

   //Pass time_sec value to the shaders
   glUniform1f(Uniforms::UniformLocs::time, time_sec);
}

void Scene::ReloadShader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

   if (new_shader == -1) // loading failed
   {
      DebugBreak(); //alert user by breaking and showing debugger
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if (shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;
   }
}

//Initialize OpenGL state. This function only gets called once.
void Scene::Init()
{
   glewInit();
   RegisterDebugCallback();

   //Print out information about the OpenGL version supported by the graphics driver.	
   std::ostringstream oss;
   oss << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
   oss << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
   oss << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
   oss << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

   int max_uniform_block_size = 0;
   glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size);
   oss << "GL_MAX_UNIFORM_BLOCK_SIZE: " << max_uniform_block_size << std::endl;

   int max_storage_block_size = 0;
   glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_storage_block_size);
   oss << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE: " << max_storage_block_size << std::endl;

   int max_texture_size = 0;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
   oss << "GL_MAX_TEXTURE_SIZE: " << max_texture_size << std::endl;

   int max_3d_texture_size = 0;
   glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_3d_texture_size);
   oss << "GL_MAX_3D_TEXTURE_SIZE: " << max_3d_texture_size << std::endl;

   //Output to console and file
   std::cout << oss.str();

   std::fstream info_file("info.txt", std::ios::out | std::ios::trunc);
   info_file << oss.str();
   info_file.close();
   glEnable(GL_DEPTH_TEST);

   ReloadShader();

 #pragma region VBO and VBO creation
   const int n = 50;
   surface = create_indexed_surf_interleaved_strip_vao(n);
#pragma endregion

   if (NUM_INSTANCES > 1) {

       glBindVertexArray(surface.vao);

       glm::vec4 color_data[NUM_INSTANCES] = {};
       for (int i = 0; i < NUM_INSTANCES; i++) {
           color_data[i] = glm::vec4((i / 3)/2.0, (i % 3)/2.0, ((i + 1) / 3.0), 1.0);
       }

       int color_loc = glGetAttribLocation(shader_program, "color");
       int matrix_loc = glGetAttribLocation(shader_program, "model_matrix");

       GLuint color_buffer;
       glGenBuffers(1, &color_buffer);
       glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
       glBufferData(GL_ARRAY_BUFFER,NUM_INSTANCES * sizeof(glm::vec4), color_data,GL_STATIC_DRAW);

       glVertexAttribPointer(color_loc, 4, GL_FLOAT,GL_FALSE, 0, 0);
       glEnableVertexAttribArray(color_loc);
       glVertexAttribDivisor(color_loc, 1);

   
       glm::mat4 modmatric_data[NUM_INSTANCES] = {};
       for (int n = 0; n < NUM_INSTANCES; n++)
       {
           float a = 30 * (n / 3 - 1);
           float b = 30 * (n % 3 - 1);
           float c = 0;
           modmatric_data[n] = glm::translate(glm::vec3(a, b, c));
}

       GLuint model_matrix_buffer;
       glGenBuffers(1, &model_matrix_buffer);
       glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
       glBufferData(GL_ARRAY_BUFFER, NUM_INSTANCES * sizeof(glm::mat4), modmatric_data, GL_STATIC_DRAW);
       // Loop over each column of the matrix...
       for (int i = 0; i < 4; i++)
       {
           // Set up the vertex attribute
           glVertexAttribPointer(matrix_loc + i,              // Location
               4, GL_FLOAT, GL_FALSE,       // vec4
               sizeof(glm::mat4),                // Stride
               (void*)(sizeof(glm::vec4) * i)); // Start offset
           // Enable it
           glEnableVertexAttribArray(matrix_loc + i);
           // Make it instanced
           glVertexAttribDivisor(matrix_loc + i, 1);
       }
       glBindVertexArray(0);
   }

   if (surface.mode == GL_TRIANGLE_STRIP) {
       glEnable(GL_PRIMITIVE_RESTART);
       glPrimitiveRestartIndex(0xFFFFFFFF);
   }

   glPointSize(5.0f);
   Uniforms::SceneData.eye_w = glm::vec4(0.0f, 3.0f, 1.0f, 1.0f);

   Camera::UpdateP();
   Uniforms::Init();
}