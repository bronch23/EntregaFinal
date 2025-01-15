
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm.hpp>
#include <Color.hpp>
#include <Color_Buffer.hpp>
#include <glad/glad.h>

namespace udit
{
    class Scene
    {
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

        void update();
        void render();
        void resize(int width, int height);

    private:

        std::vector<Mesh> meshes;
        GLuint program_id;
        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;
        GLuint normal_matrix_id;  // Se añade para las normales
        float angle;

        void load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path);
        GLuint create_texture_2d(const std::string& texture_path);
        GLuint compile_shaders();
    };
}


