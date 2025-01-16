#pragma once
#include "MeshNode.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace udit
{
    void MeshNode::add_mesh(const Mesh& mesh)
    {
        meshes.push_back(mesh);
    }

    void MeshNode::draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id)
    {
        // Activar el programa de shaders
        glUseProgram(program_id);

        for (const auto& mesh : meshes)
        {
            glBindVertexArray(mesh.vao_id);

            // Calcular matrices necesarias
            glm::mat4 model_view_matrix = view_matrix * mesh.model_matrix;
            glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

            // Configurar las matrices en el shader
            glUniformMatrix4fv(glGetUniformLocation(program_id, "model_view_matrix"), 1, GL_FALSE, glm::value_ptr(model_view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(program_id, "normal_matrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.texture_id);
            glUniform1i(glGetUniformLocation(program_id, "texture_sampler"), 0);

            // Dibujar la malla
            glDrawElements(GL_TRIANGLES, mesh.number_of_indices, GL_UNSIGNED_SHORT, 0);
        }

        // Dibujar los hijos del nodo
        for (const auto& child : children)
        {
            child->draw(view_matrix, projection_matrix, program_id);
        }

        // Desactivar el VAO
        glBindVertexArray(0);
    }
}