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
        };

        std::vector<Mesh> meshes;

        MeshNode() = default;

        void add_mesh(const Mesh& mesh);

        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override;
    };
}

