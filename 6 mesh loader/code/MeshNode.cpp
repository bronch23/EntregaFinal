// Proyecto: Entrega Programación Gráfica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripción: Implementación de conceptos básicos de programación de gráficos
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

    void MeshNode::update_meshes(float delta_time)
    {
        for (auto& mesh : meshes)
        {
            if (mesh.rotate_y)
            {
                mesh.model_matrix = glm::rotate(
                    mesh.model_matrix,
                    delta_time * mesh.rotation_speed, // Velocidad de rotación ajustada por tiempo
                    glm::vec3(0.f, 0.f, 1.f)         // Rotación sobre el eje Y
                );
            }
        }
    }

    void MeshNode::set_mesh_transparency(size_t index, float transparency)
    {
        if (index < meshes.size())
        {
            meshes[index].transparency = glm::clamp(transparency, 0.0f, 1.0f);
        }
    }

    void MeshNode::draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id)
    {
        glUseProgram(program_id);

        for (const auto& mesh : meshes)
        {
            glBindVertexArray(mesh.vao_id);

            glm::mat4 model_view_matrix = view_matrix * mesh.model_matrix;
            glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

            glUniformMatrix4fv(glGetUniformLocation(program_id, "model_view_matrix"), 1, GL_FALSE, glm::value_ptr(model_view_matrix));
            glUniformMatrix4fv(glGetUniformLocation(program_id, "normal_matrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.texture_id);
            glUniform1i(glGetUniformLocation(program_id, "texture_sampler"), 0);

            // Configurar transparencia
            glUniform1f(glGetUniformLocation(program_id, "transparency"), mesh.transparency);

            // Activar blending si hay transparencia
            if (mesh.transparency < 1.0f)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            glDrawElements(GL_TRIANGLES, mesh.number_of_indices, GL_UNSIGNED_SHORT, 0);

            // Desactivar blending si estaba activado
            if (mesh.transparency < 1.0f)
            {
                glDisable(GL_BLEND);
            }
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }
}