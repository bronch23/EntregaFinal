#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
namespace udit
{
    class SceneNode
    {
    public:
        using Ptr = std::shared_ptr<SceneNode>;

        // Transformaciones básicas
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f); // En grados
        glm::vec3 scale = glm::vec3(1.0f);

        // Lista de hijos
        std::vector<Ptr> children;

        // Constructor
        SceneNode() = default;

        // Añadir un nodo hijo
        void add_child(Ptr child)
        {
            children.push_back(std::move(child));
        }
        const std::vector<std::shared_ptr<SceneNode>>& get_children() const { return children; }
        // Actualizar transformaciones recursivamente
        void update(const glm::mat4& parent_transform = glm::mat4(1.0f))
        {
            // Calcular la transformación local
            glm::mat4 local_transform = glm::translate(glm::mat4(1.0f), position);
            local_transform = glm::rotate(local_transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
            local_transform = glm::rotate(local_transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
            local_transform = glm::rotate(local_transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
            local_transform = glm::scale(local_transform, scale);

            // Transformación global
            glm::mat4 global_transform = parent_transform * local_transform;

            // Actualizar todos los hijos
            for (auto& child : children)
            {
                child->update(global_transform);
            }
        }

        // Dibujar el nodo y sus hijos (virtual para personalización)
        virtual void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id)
        {
            for (auto& child : children)
            {
                child->draw(view_matrix, projection_matrix, program_id);
            }
        }
    };
}
