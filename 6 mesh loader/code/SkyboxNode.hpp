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
        // Constructor que inicializa el Skybox y asocia la c�mara
        SkyboxNode(const std::string& cubemap_path, std::shared_ptr<CameraNode> camera_node)
            : skybox(cubemap_path), camera_node(camera_node) {}

        // Sobreescribir el m�todo draw
        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override
        {
            if (camera_node)
            {
                glm::mat4 skybox_view_matrix = glm::mat4(glm::mat3(camera_node->get_view_matrix()));
                skybox.render(skybox_view_matrix, projection_matrix);
            }
        }
    };
}




