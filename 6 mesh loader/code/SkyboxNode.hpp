#pragma once
#include "SceneNode.hpp"
#include "Skybox.hpp"
#include "Camera.hpp"

namespace udit
{
    class SkyboxNode : public SceneNode
    {
    private:
        Skybox skybox;
        const Camera& camera;

    public:
        SkyboxNode(const std::string& texture_base_path, const Camera& cam)
            : skybox(texture_base_path), camera(cam) {
        }

        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override
        {
            // Renderizar el Skybox utilizando la cámara
            glDepthMask(GL_FALSE);
            skybox.render(camera);
            glDepthMask(GL_TRUE);
        }
    };
}




