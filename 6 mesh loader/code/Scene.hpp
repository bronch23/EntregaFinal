
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm.hpp>
#include <Color.hpp>
#include <Color_Buffer.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Skybox.hpp"
#include "SceneNode.hpp"
#include "MeshNode.hpp"
#include "CameraNode.hpp"
#include "FrameBuffer.hpp"
#include "json.hpp"
#include "ElevationNode.hpp"
#include "LightNode.hpp"
#include "SkyboxNode.hpp"
namespace udit
{
    class Scene
    {
        using json = nlohmann::json;
    public:

        struct Mesh
        {
            glm::mat4 model_matrix;
            GLuint vao_id;
            GLuint vbo_ids[3];
            GLuint texture_id;
            GLsizei number_of_indices;
        };

        Scene(int width, int height);
        ~Scene();

        void initialize_default_scene();
        void update();
        void render();
        void resize(int width, int height);
        SceneNode::Ptr root_node;
        int    width;
        int    height;
        void on_click(int pointer_x, int pointer_y, bool down);
        void on_drag(int pointer_x, int pointer_y);
        void initialize_screen_quad();
        void save_scene(const std::string& file_path);
        void load_scene(const std::string& file_path);
        void load_camera(const json& camera_data);
        void load_meshes(const json& meshes_data);
        void load_elevation(const json& elevation_data);
        void load_lights(const json& lights_data);
        void load_skybox(const json& skybox_data);
        void load_postprocessing(const json& postprocessing_data);
        std::shared_ptr<MeshNode> mesh_node; 
        std::shared_ptr<ElevationMeshNode> elevation_node;
        std::shared_ptr<LightNode> light_node;
        std::shared_ptr<SkyboxNode> skybox_node;

    private:

        std::vector<Mesh> meshes;
        GLuint program_id;
        GLuint screen_shader_program;
        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;
        GLuint normal_matrix_id;  // Se añade para las normales
        float angle;

        /*MeshNode::Mesh create_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path);*/
        GLuint create_texture_2d(const std::string& texture_path);
        GLuint compile_shaders();
       
        GLuint compile_screen_shaders();
        float postprocess_effect_intensity = 0.5f;
        glm::vec3 postprocess_color_adjustment = { 1.2f, 1.0f, 0.8f };

        Framebuffer framebuffer; // Framebuffer para renderizar la escena
        GLuint screen_quad_vao;  // VAO para renderizar el framebuffer en pantalla

        Camera camera;
        std::shared_ptr<CameraNode> camera_node;
        Skybox skybox;
        // Variables para controlar la cámara
        float angle_around_x, angle_around_y;
        float angle_delta_x, angle_delta_y;
        bool pointer_pressed;
        int last_pointer_x, last_pointer_y;
    };
}


