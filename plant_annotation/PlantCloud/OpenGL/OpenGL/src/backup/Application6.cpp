#pragma comment(lib, "glu32.lib") 


#define _USE_MATH_DEFINES
#include <Windows.h>
#include <commdlg.h>
#include <GL/glew.h>


#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <iostream>
#include<fstream>
#include<sstream>
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui_impl_glfw.h"
#include <imgui\imgui_impl_opengl3.h>
#include "PointCloud.h"
#include <numeric>
#include <math.h>
#include "Icosphere.h"
#include <algorithm>
#include <climits>
#include "Sphere.h"
#include <glm/gtx/string_cast.hpp>

std::string OpenFile(const char* filter, GLFWwindow* window)
{
    OPENFILENAMEA ofn;
    CHAR szfile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = glfwGetWin32Window(window);
    ofn.lpstrFile = szfile;
    ofn.nMaxFile = sizeof(szfile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == true)
    {
        return ofn.lpstrFile;
    }
    return std::string();
}


std::string SaveFile(const char* filter, GLFWwindow* window)
{
    OPENFILENAMEA ofn;
    CHAR szfile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = glfwGetWin32Window(window);
    ofn.lpstrFile = szfile;
    ofn.nMaxFile = sizeof(szfile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetSaveFileNameA(&ofn) == true)
    {
        return ofn.lpstrFile;
    }
    return std::string();
}


static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
{
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();
    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    if (ImGui::Button("X", buttonSize))
        values.x = resetValue;
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    // SliderFloat
    ImGui::SliderFloat("##X", &values.x, -1, 1);

    //ImGui::DragFloat("##X", &values.x, 0.1f);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    if (ImGui::Button("Y", buttonSize))
        values.y = resetValue;
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    if (ImGui::Button("Z", buttonSize))
        values.z = resetValue;
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f);
    ImGui::PopItemWidth();


    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

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
    int screenWidth = 1920;
    int screenHeight = 1080;
    window = glfwCreateWindow(screenWidth, screenHeight, "Plant Annotation", NULL, NULL);

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

        ///////////////////////////////////// main point cloud
        // Vertex array and vertex buffer
        VertexArray va;
        VertexBuffer vb(&pc.GetPoints()[0], pc.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        va.AddBuffer(vb, layout);
        //adding another vertex buffer for color info
        // Index buffer
       /* std::vector<int> indices(pc.GetNumPoints());
        int pc_num_points = pc.GetNumPoints();
        for (int i = 0; i < pc_num_points; i++) indices[i] = i;*/
        std::vector<unsigned int> indices = pc.GetIndices();
        IndexBuffer ib(&indices[0], pc.GetTotalNumIndices());

        ///////////////////////////////////// sphere
        Sphere sphere(0.1f, 5, true);
        VertexArray va2;
        VertexBuffer vb2(&sphere.GetPoints()[0], sphere.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
        VertexBufferLayout layout2;
        layout2.Push<float>(3);
        layout2.Push<float>(3);
        va2.AddBuffer(vb2, layout2);
        //adding another vertex buffer for color info
        // Index buffer
       /* std::vector<int> indices(pc.GetNumPoints());
        int pc_num_points = pc.GetNumPoints();
        for (int i = 0; i < pc_num_points; i++) indices[i] = i;*/
        std::vector<unsigned int> indices2 = sphere.GetIndices();
        IndexBuffer ib2(&indices2[0], sphere.GetTotalNumIndices());

        //Shader
        std::string sh_filepath = "res/shaders/Basic.shader";
        Shader sh(sh_filepath);
        sh.Bind();
        //sh.SetUniform4f("u_Color", 0.7f, 0.1f, 0.3f, 1.0f);

        // Model view projection matrices
        glm::vec3 translation(0.0f, 0.0f, 0.0f);
        glm::vec3 ball_translation(0.0f, 0.0f, 0.0f);
        glm::vec3 rotation(1.0f, 0.0f, 0.0f);
        float angle = 270;
        float zoom_factor = 5.0f;

        float ann_rad = 0.1f;
        float ann_rad_old = ann_rad;
        float x_rot_angle = 0.0f;
        float y_rot_angle = 0.0f;
        float prev_mouse_x = 0.0f;
        float prev_mouse_y = 0.0f;
        float x_trans = 0.0f;
        float y_trans = 0.0f;
        //glm::mat4 proj = glm::ortho(pc.Getlowerbb().x - 1, pc.Getupperbb().x + 1, pc.Getlowerbb().y - 1, pc.Getupperbb().y + 1, pc.Getlowerbb().z - 2, pc.Getupperbb().z + 2);
        //glm::mat4 proj = glm::ortho(-1.0f, +1.0f, 1.0f, +1.0f, -2.0f, 2.0f);
        //glm::mat4 proj = glm::ortho(-12.0f - (-zoom_factor), +12.0f + (-zoom_factor), -12.0f - (-zoom_factor), +12.0f + (-zoom_factor), -1.0f, 1.0f);
        //glm::mat4 proj = glm::ortho(-16.0f*(zoom_factor*0.1f), +16.0f* (zoom_factor * 0.1f), -9.0f* (zoom_factor * 0.1f), +9.0f* (zoom_factor * 0.1f), -1.0f, 1.0f);
        glm::mat4 proj = glm::ortho(-16.0f - (-zoom_factor * 0.1f), +16.0f + (-zoom_factor * 0.1f), -9.0f - (-zoom_factor * 0.1f), +9.0f + (-zoom_factor * 0.1f), -1.0f, 1.0f);

        //glm::mat4 proj = glm::ortho(-12.0f , +12.0f , -12.0f , +12.0f , -1.0f, 1.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        // unbind everything
        GLCall(glUseProgram(0));
        GLCall(glBindVertexArray(0));
        sh.Unbind();
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        va2.Unbind();
        vb2.Unbind();
        ib2.Unbind();

        // setup imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
        bool show_demo_window = true;
        bool show_another_window = false;
        bool erase_enabled = false;

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        ImVec4 label_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);

        glm::vec4 pcenter_sphere = glm::vec4(0.0f, 0.0f, 0.1f, 1.0f);

        glEnable(GL_PROGRAM_POINT_SIZE);

        //Renderer initialization
        Renderer renderer;
        /* Icosphere sphere(1.0f, 5, true);*/
         //glEnable(GL_DEPTH_TEST);

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
            std::cout << show_another_window << std::endl;
            if (show_another_window || erase_enabled)
            {
                float x_extents = (pc.Getupperbb().x + 1) - (pc.Getlowerbb().x - 1);
                float y_extents = (pc.Getupperbb().y + 1) - (pc.Getlowerbb().y - 1);
                //x_trans += (x_extents * (io.MousePos[0] - prev_mouse_x)/1280.0f);
                //y_trans -= (y_extents * (io.MousePos[1] - prev_mouse_y)/720.0f);
                x_trans = (pc.Getlowerbb().x - 1) + (x_extents * (io.MousePos[0] - 0) / 1920.0f);
                y_trans = -((pc.Getlowerbb().y - 1) + (y_extents * (io.MousePos[1] - 0) / 1080.0f));
                //std::cout << io.MousePos[0] << " " << io.MousePos[1] << std::endl;
                ball_translation = glm::vec3(x_trans, y_trans, 0.0f);
            }
            else {
                ball_translation = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            zoom_factor += io.MouseWheel;
            zoom_factor = (zoom_factor > 160.0) ? 160.0f : zoom_factor;
            zoom_factor = (zoom_factor <= 1.0) ? 1.0f : zoom_factor;

            // glClear(GL_COLOR_BUFFER_BIT);
            //int display_w, display_h;
            //glfwGetFramebufferSize(window, &display_w, &display_h);
            //glViewport(0, 0, display_w, display_h);

            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);

            float cx1 = pc.Getlowerbb().x - zoom_factor; // -1.68156874
            float cy1 = pc.Getlowerbb().y - zoom_factor; // -1.53836489
            float cx2 = pc.Getupperbb().x + zoom_factor; // 1.62599421
            float cy2 = pc.Getupperbb().y + zoom_factor; // 1.70320010
            float cz1 = pc.Getlowerbb().z - 2; //  -2.09999990
            float cz2 = pc.Getupperbb().z + 2; // 3.33530617


            //proj = glm::ortho(pc.Getlowerbb().x - zoom_factor, pc.Getupperbb().x + zoom_factor, pc.Getlowerbb().y - zoom_factor, pc.Getupperbb().y + zoom_factor, pc.Getlowerbb().z - 2, pc.Getupperbb().z + 2);
              //proj = glm::ortho(-2.0f, +2.0f, -2.0f, +2.0f, -4.0f, 4.0f );
              //proj = glm::ortho(-12.0f-(-zoom_factor), +12.0f + (-zoom_factor), -12.0f - (-zoom_factor), +12.0f + (-zoom_factor),  -1.0f, 1.0f);
             //proj = glm::ortho(-12.0f , +12.0f , -12.0f , +12.0f , -1.0f, 1.0f);
              //glm::ortho(-16.0f * (zoom_factor * 0.1f), +16.0f * (zoom_factor * 0.1f), -9.0f * (zoom_factor * 0.1f), +9.0f * (zoom_factor * 0.1f), -1.0f, 1.0f);

              //proj = glm::ortho(-32.0f * (zoom_factor), +32.0f * (zoom_factor), -18.0f * (zoom_factor), +18.0f * (zoom_factor ), -1.0f, 1.0f);

              // proj = glm::ortho(-16.0f * zoom_factor, +16.0f * zoom_factor, -9.0f * zoom_factor, +9.0f * zoom_factor, -1.0f, 1.0f);

              //proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -3.0f, 4.0f);

              //proj = glm::ortho(cx1, cx2, cy1, cy2, cz1, cz2);
            //proj = glm::ortho(-1.68156874f, 1.62599421f, -1.53836489f, 1.70320010f, -2.09999990f, 3.33530617f);
            //proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -3.0f, 4.0f);

            //proj = glm::ortho(-32.0f * (zoom_factor * 0.1f), +32.0f * (zoom_factor * 0.1f), -18.0f * (zoom_factor * 0.1f), +18.0f * (zoom_factor * 0.1f), -1.0f, 1.0f);

           // proj = glm::ortho(-32.0f -(-zoom_factor), +32.0f + (-zoom_factor), -18.0f - (-zoom_factor), +18.0f + (-zoom_factor),  -1.0f, 1.0f);

            proj = glm::ortho(-16.0f * (zoom_factor * 0.1f), +16.0f * (zoom_factor * 0.1f), -9.0f * (zoom_factor * 0.1f), +9.0f * (zoom_factor * 0.1f), -1.0f, 1.0f);
            // proj = glm::ortho(-16.0f -(-zoom_factor*0.1f), +16.0f + (-zoom_factor * 0.1f), -9.0f - (-zoom_factor * 0.1f), +9.0f + (-zoom_factor * 0.1f),  -1.0f, 1.0f);

            {
                float x_rot_axis = (pc.Getupperbb().x + pc.Getlowerbb().x) / 2.0f;
                float y_rot_axis = (pc.Getupperbb().y + pc.Getlowerbb().y) / 2.0f;
                float z_rot_axis = (pc.Getupperbb().z + pc.Getlowerbb().z) / 2.0f;

                glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
                model = glm::rotate(model, x_rot_angle * (float)M_PI / 180, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, y_rot_angle * (float)M_PI / 180, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, angle * (float)M_PI / 180, rotation); // rotation to align z axis vertically

                //model = glm::rotate(model, angle * (float)M_PI / 180, rotation);

                glm::mat4 mvp = proj * view * model;

                //std::cout << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << std::endl;
                //pc.do_transform( mvp);
                // get the mouse point 
                float x_extents = (pc.Getupperbb().x + 1) - (pc.Getlowerbb().x - 1);
                float y_extents = (pc.Getupperbb().y + 1) - (pc.Getlowerbb().y - 1);
                float x_pos = (x_extents * (io.MousePos[0] / 1920.0f));
                float y_pos = (y_extents * (-io.MousePos[1] / 1080.0f));
                glm::vec4 curr_pos = glm::vec4(x_pos, y_pos, -1.0f, 1.0f);
                // do inverse transform // get the resulting point x and y coords
                glm::vec4 curr_pos_inv = glm::inverse(proj) * curr_pos;

                curr_pos_inv = glm::vec4(curr_pos_inv[0], curr_pos_inv[1], -1.0f, 0.0f);
                curr_pos_inv = glm::inverse(view) * curr_pos_inv;
                curr_pos_inv = glm::vec4(curr_pos_inv[0], curr_pos_inv[1], -1.0f, 0.0f);
                curr_pos_inv = glm::inverse(model) * curr_pos_inv;

                // step 1: normalized device coords
                float mouse_x = io.MousePos[0]; float width = 1920.0f;
                float mouse_y = io.MousePos[1]; float height = 1080.0f;
                float x = (2.0f * mouse_x) / width - 1.0f;
                float y = 1.0f - (2.0f * mouse_y) / height;
                float z = 1.0f;
                glm::vec3 ray_nds = glm::vec3(x, y, z);

                //step 2: homogenous clip coords
                glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

                //step 3: Eye coords
               // glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;

                //step 3: Eye coords
                glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
                ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

                //step 4: world coords
                glm::vec3 ray_wor = (glm::inverse(model) * ray_eye);
                // don't forget to normalise the vector at some point
                ray_wor = glm::normalize(ray_wor);


                // std::cout  << glm::to_string(curr_pos_inv) <<std::endl;
                // get all points with in 0.4 radius (find euclidean distance w.r.t to x and y coords )
                // color those points // update the color values in point clouds
                if (show_another_window) {
                    curr_pos_inv = glm::normalize(curr_pos_inv);
                    //pc.do_color_small(curr_pos_inv[0], curr_pos_inv[1]);
                    //pc.do_color_small(ray_wor[0], ray_wor[1]);
                    pc.do_color_small(pcenter_sphere[0], pcenter_sphere[1], mvp, ann_rad, label_color.x * label_color.w, label_color.y * label_color.w, label_color.z * label_color.w);

                    // pc.do_color_small(x_pos, y_pos, mvp);

                    std::vector<glm::vec3>  pts = pc.GetPoints();
                    //std::cout  << glm::to_string(pts[0]) << glm::to_string(pts[1]) <<std::endl;
                    vb.RebindBuffer(&pc.GetPoints()[0], pc.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
                }

                else if (erase_enabled) {
                    curr_pos_inv = glm::normalize(curr_pos_inv);
                    //pc.do_color_small(curr_pos_inv[0], curr_pos_inv[1]);
                    //pc.do_color_small(ray_wor[0], ray_wor[1]);
                    pc.do_erase(pcenter_sphere[0], pcenter_sphere[1], mvp, ann_rad);

                    // pc.do_color_small(x_pos, y_pos, mvp);

                    std::vector<glm::vec3>  pts = pc.GetPoints();
                    //std::cout  << glm::to_string(pts[0]) << glm::to_string(pts[1]) <<std::endl;
                    vb.RebindBuffer(&pc.GetPoints()[0], pc.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
                }

                //std::cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;

                //glm::mat4 ident = glm::mat4(1.0f);
                sh.SetUniform4fMat("u_MVP", mvp);

                sh.Bind();
                /* Render here */
                renderer.clear();
                renderer.Draw(va, ib, sh);
                va.Unbind();
                ib.Unbind();
            }

            //////////////////////////////////////////////////////////////// draw sphere
            {
                if (ann_rad != ann_rad_old) {
                    ann_rad_old = ann_rad;
                    Sphere* sph = &sphere;
                    //delete[] sph;
                    Sphere sphere(ann_rad, 5, true);
                    vb2.RebindBuffer(&sphere.GetPoints()[0], sphere.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
                    //sphere.ReInitialize(ann_rad);
                    //vb.RebindBuffer(&pc.GetPoints()[0], pc.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
                }

                glm::mat4 view = glm::mat4(1.0f);
                glm::mat4 model = glm::translate(glm::mat4(1.0f), ball_translation);
                glm::mat4 mvp = proj * view * model;

                ///
                glm::vec4 center_sphere = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                pcenter_sphere = mvp * center_sphere;

                sh.SetUniform4fMat("u_MVP", mvp);
                // renderer.clear();
                renderer.Draw(va2, ib2, sh);
            }
            //////////////////////////////////////////////////////////////////////////////////////

                       // update mouse_position
            prev_mouse_x = io.MousePos[0];
            prev_mouse_y = io.MousePos[1];

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.

                //if (ImGui::Begin("Example: Simple layout", nullptr, ImGuiWindowFlags_MenuBar))
                //{
                    //IMGUI_DEMO_MARKER("Examples/Simple layout");
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
                        if (ImGui::MenuItem("Open"))
                        {
                            std::string filepath = OpenFile("Pointcloud (*.txt)\0*.txt\0", window);
                            if (!filepath.empty()) {
                                pc.ReInitialize(filepath);
                                vb.RebindBuffer(&pc.GetPoints()[0], pc.GetTotalNumPoints() * 2 * sizeof(glm::vec3));
                            }
                        }
                        if (ImGui::MenuItem("Save"))
                        {
                            std::string filepath = SaveFile("Pointcloud (*.txt)\0*.txt\0", window);
                            if (!filepath.empty()) {
                                int letssavefile = 0;
                            }
                        }// *p_open = false;
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }


                //std::cout << ImGui::GetScrollY() << std::endl;
                //std::cout << io.MouseWheel << std::endl;


                //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Enable annotation", &show_another_window);
                ImGui::Checkbox("Enable eraser", &erase_enabled);

                //  ImGui::SliderFloat2("translation (x,y)", &translation.x, pc.Getlowerbb().x - 1, pc.Getupperbb().x + 1);            // Edit 1 float using a slider from 0.0f to 1.0f
                DrawVec3Control("Translation", translation);
                DrawVec3Control("Rotation", rotation);


                //ImGui::DragFloat("zoom_factor", &zoom_factor, 0.001f, 0.01f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                                                                                                                                   //ImGui::SliderFloat("translation z", &translation.z, pc.Getlowerbb().z - 1, pc.Getupperbb().z + 1);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("zoom_factor", &zoom_factor, 1.0f, 90.0f, "%.3f");            // Edit 1 float using a slider from 0.0f to 1.0f

                //ImGui::SliderFloat("angle x", &angle, 0, 360);            // Edit 1 float using a slider from 0.0f to 1.0f

                ImGui::ColorEdit3("Background color", (float*)&clear_color); // Edit 3 floats representing a color
                ImGui::SliderFloat("Annotation tool radius", &ann_rad, 0, 1);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("Label color", (float*)&label_color); // Edit 3 floats representing a color

                //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                //    counter++;
                //ImGui::SameLine();
                //ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
                //ImGui::ShowDemoWindow();
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