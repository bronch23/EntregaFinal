#pragma once

#include "SceneNode.hpp"
#include "Camera.hpp"

namespace udit
{ 
class CameraNode : public SceneNode
{
private:
    Camera camera;

public:
    CameraNode(float aspect_ratio)
        : camera(aspect_ratio) {
    }

    // Métodos de transformación y proyección
    glm::mat4 get_view_matrix() const
    {
        return camera.get_transform_matrix_inverse();
    }

    glm::mat4 get_projection_matrix() const
    {
        return camera.get_projection_matrix();
    }
    glm::mat4 get_transform_matrix_inverse()
    {
        return camera.get_transform_matrix_inverse();
    }
    float get_ratio()
    {
        return camera.get_ratio();
    }
    float get_fov()
    {
        return camera.get_fov();
    }
    // Métodos de rotación y posición
    void add_rotation_delta(float delta_x, float delta_y)
    {
        camera.add_rotation_delta(delta_x, delta_y);
    }

    void apply_rotation()
    {
        camera.apply_rotation();
    }

    void set_position(float x, float y, float z)
    {
        camera.set_location(x, y, z);
    }

    void move(const glm::vec3& delta)
    {
        camera.move(delta);
    }

    void set_fov(float new_fov) 
    { 
        camera.set_fov(new_fov);
    }

    glm::vec3 get_position() const
    {
        return camera.get_location();
    }

    void reset(float fov_degrees, float near_z, float far_z, float ratio)
    {
        camera.reset(fov_degrees, near_z, far_z, ratio);
    }

    void set_ratio(float newRatio)
    {
        camera.set_ratio(newRatio);
    }

    // Métodos de actualización y herencia
    void update(const glm::mat4& parent_transform = glm::mat4(1.0f)) override
    {
        // Calcula la transformación local usando SceneNode
        SceneNode::update(parent_transform);

        // Utiliza la transformación global calculada para la cámara
        glm::mat4 global_transform = parent_transform * glm::translate(glm::mat4(1.0f), position);
        
        camera.apply_rotation(); // Aplica cualquier rotación pendiente de la cámara
    }
};
}
