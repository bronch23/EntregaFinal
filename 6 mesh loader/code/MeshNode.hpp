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
            glm::mat4 model_matrix;
            GLuint vao_id;
            GLuint vbo_ids[4];
            GLuint texture_id;
            GLsizei number_of_indices;
            bool rotate_y = false; // Nueva variable para determinar si rota
            float rotation_speed = 0.01f; // Velocidad de rotación
        };

        std::vector<Mesh> meshes;

        MeshNode() = default;

        void add_mesh(const Mesh& mesh);
        void update_meshes(float delta_time);
        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override;
    };
}

