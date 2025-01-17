// Proyecto: Entrega Programación Gráfica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripción: Implementación de conceptos básicos de programación de gráficos
#pragma once
#include "SceneNode.hpp"
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

namespace udit
{
    class LightNode : public SceneNode
    {
    private:
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };  // Inicializado por defecto
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };     // Color blanco por defecto
        float ambient_intensity{ 0.2f };         // Intensidad ambiental inicial
        float diffuse_intensity{ 0.8f };         // Intensidad difusa inicial

    public:
        LightNode(GLuint program_id) 
        {
            GLint light_position = glGetUniformLocation(program_id, "light.position");
            GLint light_color = glGetUniformLocation(program_id, "light.color");
            GLint ambient_intensity_loc = glGetUniformLocation(program_id, "ambient_intensity");
            GLint diffuse_intensity_loc = glGetUniformLocation(program_id, "diffuse_intensity");

            // Cargar los valores al shader
            glUniform3fv(light_position, 1, glm::value_ptr(position));
            glUniform3fv(light_color, 1, glm::value_ptr(color));
            glUniform1f(ambient_intensity_loc, ambient_intensity);
            glUniform1f(diffuse_intensity_loc, diffuse_intensity);
        }

        void set_position(const glm::vec3& new_position)
        {
            position = new_position;
        }

        void set_color(const glm::vec3& new_color)
        {
            color = new_color;
        }

        void set_ambient_intensity(float ambient)
        {
            ambient_intensity = ambient;
        }

        void set_diffuse_intensity(float diffuse)
        {
            diffuse_intensity = diffuse;
        }

        glm::vec3 get_position() const
        {
            return position;
        }

        glm::vec3 get_color() const
        {
            return color; 
        }

        float get_ambient_intensity() const
        {
            return ambient_intensity; 
        }

        float get_diffuse_intensity() const
        {
            return diffuse_intensity; 
        }

        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override
        {

            GLint light_position = glGetUniformLocation(program_id, "light.position");
            GLint light_color = glGetUniformLocation(program_id, "light.color");
            GLint ambient_intensity_loc = glGetUniformLocation(program_id, "ambient_intensity");
            GLint diffuse_intensity_loc = glGetUniformLocation(program_id, "diffuse_intensity");

            // Cargar los valores al shader
            glUniform3fv(light_position, 1, glm::value_ptr(position));
            glUniform3fv(light_color, 1, glm::value_ptr(color));
            glUniform1f(ambient_intensity_loc, ambient_intensity);
            glUniform1f(diffuse_intensity_loc, diffuse_intensity);
        }
    };
}

