#pragma once

#include "SceneNode.hpp"
#include <glad/glad.h>
#include <glm.hpp>
#include <vector>

namespace udit
{
    class MeshNode : public SceneNode
    {
    public:
        struct Mesh
        {
            GLuint vao_id;
            GLuint vbo_ids[4];
            GLuint texture_id;
            GLsizei number_of_indices;
            glm::mat4 model_matrix;
            float rotation_speed = 1.0f;
            bool rotate_y = false;

            float transparency = 1.0f; // Nuevo atributo para transparencia (1.0 = opaco, 0.0 = completamente transparente)
        };

        std::vector<Mesh> meshes;

        MeshNode() = default;
        void set_mesh_transparency(size_t index, float transparency);
        void add_mesh(const Mesh& mesh);
        void update_meshes(float delta_time);
        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override;
    };
}

