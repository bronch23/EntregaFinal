
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
    struct Mesh
    {
        GLuint vao_id;
        GLuint vbo_ids[3];
        GLuint texture_id; // Identificador de textura
        glm::mat4 model_matrix;
        size_t number_of_indices;
    };

    class Scene
    {
    private:
        std::vector<Mesh> meshes;
        GLuint program_id;
        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;

        float angle;

    public:
        Scene(int width, int height);
        ~Scene();

        void update();
        void render();
        void resize(int width, int height);

    private:
        GLuint create_texture_2d(const std::string& texture_path);
        void load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path);
        GLuint compile_shaders();
    };
}


