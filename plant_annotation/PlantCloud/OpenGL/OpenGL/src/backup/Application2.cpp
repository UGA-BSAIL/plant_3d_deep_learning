#pragma comment(lib, "glu32.lib") 
#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include<fstream>
#include<sstream>
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include <imgui\imgui_impl_opengl3.h>
#include "PointCloud.h"
#include <numeric>
#include <math.h>
#include "Sphere.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    if (glewInit() != GLEW_OK)
        std::cout << "GLewInit Error!" << std::endl;
    {
        // PointCloud initialization
        std::string pc_filepath = "res/pointclouds/denorm.txt";
        PointCloud pc(pc_filepath);

        // Vertex array and vertex buffer
        VertexArray va;
        VertexBuffer vb(&pc.GetPoints()[0], pc.GetNumPoints() * 2 * sizeof(glm::vec3));
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        va.AddBuffer(vb, layout);
        //adding another vertex buffer for color info


        // Index buffer
        std::vector<int> indices(pc.GetNumPoints());
        int pc_num_points = pc.GetNumPoints();
        for (int i = 0; i < pc_num_points; i++) indices[i] = i;
        IndexBuffer ib(&indices[0], pc.GetNumPoints());

        //Shader
        std::string sh_filepath = "res/shaders/Basic.shader";
        Shader sh(sh_filepath);
        sh.Bind();
        //sh.SetUniform4f("u_Color", 0.7f, 0.1f, 0.3f, 1.0f);

        // Model view projection matrices
        glm::vec3 translation(0.0f, 0.0f, 0.0f);
        glm::vec3 rotation(1.0f, 0.0f, 0.0f);
        float angle = 270;
        float zoom_factor = 1;
        float x_rot_angle = 0.0f;
        float y_rot_angle = 0.0f;
        float prev_mouse_x = 0.0f;
        float prev_mouse_y = 0.0f;
        float x_trans = 0.0f;
        float y_trans = 0.0f;
        glm::mat4 proj = glm::ortho(pc.Getlowerbb().x - 1, pc.Getupperbb().x + 1, pc.Getlowerbb().y - 1, pc.Getupperbb().y + 1, pc.Getlowerbb().z - 2, pc.Getupperbb().z + 2);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        // unbind everything
        GLCall(glUseProgram(0));
        GLCall(glBindVertexArray(0));
        sh.Unbind();
        va.Unbind();
        vb.Unbind();
        ib.Unbind();

        // setup imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        glEnable(GL_PROGRAM_POINT_SIZE);

        //Renderer initialization
        Renderer renderer;
        glEnable(GL_DEPTH_TEST);

        //ImGui::SetScrollY(0.0f);
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            //std::cout << ImGuiWindowFlags_NoScrollWithMouse << std::endl;



            if (ImGui::IsMouseDragging(0))
            {
                y_rot_angle += (0.8f * (io.MousePos[0] - prev_mouse_x));
                x_rot_angle += (0.8f * (io.MousePos[1] - prev_mouse_y));
            }
            if (ImGui::IsMouseDragging(1) || ImGui::IsMouseDragging(2))
            {
                float x_extents = (pc.Getupperbb().x + 1) - (pc.Getlowerbb().x - 1);
                float y_extents = (pc.Getupperbb().y + 1) - (pc.Getlowerbb().y - 1);
                //x_trans += (x_extents * (io.MousePos[0] - prev_mouse_x)/1280.0f);
                //y_trans -= (y_extents * (io.MousePos[1] - prev_mouse_y)/720.0f);
                x_trans += (x_extents * (io.MousePos[0] - prev_mouse_x) / 1920.0f);
                y_trans -= (y_extents * (io.MousePos[1] - prev_mouse_y) / 1080.0f);

                translation = glm::vec3(x_trans, y_trans, 0.0f);
            }

            zoom_factor += io.MouseWheel;
            zoom_factor = (zoom_factor > 10) ? 10 : zoom_factor;
            zoom_factor = (zoom_factor < 0) ? 0 : zoom_factor;

            // glClear(GL_COLOR_BUFFER_BIT);
            //int display_w, display_h;
            //glfwGetFramebufferSize(window, &display_w, &display_h);
            //glViewport(0, 0, display_w, display_h);

            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);

            proj = glm::ortho(pc.Getlowerbb().x - zoom_factor, pc.Getupperbb().x + zoom_factor, pc.Getlowerbb().y - zoom_factor, pc.Getupperbb().y + zoom_factor, pc.Getlowerbb().z - 2, pc.Getupperbb().z + 2);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);

            model = glm::rotate(model, x_rot_angle * (float)M_PI / 180, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, y_rot_angle * (float)M_PI / 180, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, angle * (float)M_PI / 180, rotation); // rotation to align z axis vertically

            //model = glm::rotate(model, angle * (float)M_PI / 180, rotation);

            glm::mat4 mvp = proj * view * model;
            glm::mat4 ident = glm::mat4(1.0f);
            sh.SetUniform4fMat("u_MVP", mvp);


            /* Render here */
            renderer.clear();
            renderer.Draw(va, ib, sh);

            // update mouse_position
            prev_mouse_x = io.MousePos[0];
            prev_mouse_y = io.MousePos[1];

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                //std::cout << ImGui::GetScrollY() << std::endl;
                //std::cout << io.MouseWheel << std::endl;


                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat3("translation", &translation.x, pc.Getlowerbb().x - 1, pc.Getupperbb().x + 1);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("translation z", &translation.z, pc.Getlowerbb().z - 1, pc.Getupperbb().z + 1);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("zoom_factor", &zoom_factor, 0, 10);            // Edit 1 float using a slider from 0.0f to 1.0f

                ImGui::SliderFloat("angle x", &angle, 0, 360);            // Edit 1 float using a slider from 0.0f to 1.0f

                ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            ImGui::Render();
            //glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
            //  glClear(GL_COLOR_BUFFER_BIT);
        }
        //glDeleteProgram(shader);

    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}