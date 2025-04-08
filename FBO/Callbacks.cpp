#include "Callbacks.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <iostream>

bool isDragging = false;

float clickScreenPos[] = { 0.0,0.0 };

void Callbacks::Register(GLFWwindow* window)
{
   glfwSetKeyCallback(window, Keyboard);
   glfwSetCursorPosCallback(window, MouseCursor);
   glfwSetMouseButtonCallback(window, MouseButton);
   glfwSetFramebufferSizeCallback(window, Resize);
}

//This function gets called when a key is pressed
void Callbacks::Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   //std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if (action == GLFW_PRESS)
   {
      switch (key)
      {
      case 'r':
      case 'R':
         Scene::ReloadShader();
         break;

      case GLFW_KEY_ESCAPE:
         glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;
      }
   }
}

//This function gets called when the mouse moves over the window.
void Callbacks::MouseCursor(GLFWwindow* window, double x, double y)
{
    if (isDragging) {
        // Convert mouse position to normalized screen space (-1 to 1)
        float screenX = (2.0f * x / Scene::WindowWidth) - 1.0f;
        float screenY = 1.0f - (2.0f * y / Scene::WindowHeight); // Flip Y
        float moveX = screenX - clickScreenPos[0];
        float moveY = screenY - clickScreenPos[1];
        Scene::moveInstance(moveX,moveY);
    }
}

//This function gets called when a mouse button is pressed.
void Callbacks::MouseButton(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos); // Get cursor position
            clickScreenPos[0] = (2.0f * xpos / Scene::WindowWidth) - 1.0f;
            clickScreenPos[1] = 1.0f - (2.0f * ypos / Scene::WindowHeight);

            int readX = static_cast<int>(xpos);
            int readY = Scene::WindowHeight - static_cast<int>(ypos);
            int pickID = Scene::readPickID(readX, readY);
          
            // Output selected object
            if (pickID > 0) {
                std::cout << "Object ID Picked: " << pickID << std::endl;
                isDragging = true;
            }
            else {
                std::cout << "No object selected"  << std::endl;
                isDragging = false;
            }
        }
        else if (action == GLFW_RELEASE) {
            Scene::resetPickID(); // Reset object ID on release
            std::cout << "No object selected" << std::endl;
            isDragging = false;
        }
    }
}

void Callbacks::Resize(GLFWwindow* window, int width, int height)
{
   width = glm::max(1, width);
   height = glm::max(1, height);
   //Set viewport to cover entire framebuffer
   glViewport(0, 0, width, height);
   //Set aspect ratio used in view matrix calculation
   Scene::Camera::Aspect = float(width) / float(height);
   Scene::Camera::UpdateP();
   Scene::WindowWidth = width;
   Scene::WindowHeight = height;
   Scene::UpdateFbo();
}