// Proyecto: Entrega Programación Gráfica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripción: Implementación de conceptos básicos de programación de gráficos
#pragma once
#include "SceneNode.hpp"
#include "Skybox.hpp"
#include "CameraNode.hpp"

namespace udit
{
    class SkyboxNode : public SceneNode
    {
    private:
        Skybox skybox;
        std::shared_ptr<CameraNode> camera_node; // Cambiado a puntero compartido

    public:
        // Constructor que inicializa el Skybox y asocia la cámara
        SkyboxNode(const std::string& cubemap_path, std::shared_ptr<CameraNode> camera_node)
            : skybox(cubemap_path), camera_node(camera_node) {}

        // Sobreescribir el método draw
        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override
        {
            if (camera_node)
            {
                glm::mat4 skybox_view_matrix = glm::mat4(glm::mat3(camera_node->get_view_matrix()));
                skybox.render(skybox_view_matrix, projection_matrix);
            }
        }

        std::string get_texture_base_path() const
        {
            return skybox.get_texture_base_path();
        }
    };
}




