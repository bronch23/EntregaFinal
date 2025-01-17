// Proyecto: Entrega Programación Gráfica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripción: Implementación de conceptos básicos de programación de gráficos
#include "ElevationNode.hpp"
#include <SOIL2.h>
#include <iostream>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <stb_image.h>

namespace udit
{
    ElevationMeshNode::ElevationMeshNode(const std::string& heightmap_path, float max_height)
    {
        generate_mesh(heightmap_path, max_height);
        calculate_normals();

        // Configuración de VAO y VBO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo_positions);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    ElevationMeshNode::~ElevationMeshNode()
    {
        glDeleteBuffers(1, &vbo_positions);
        glDeleteBuffers(1, &vbo_normals);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
    void ElevationMeshNode::set_position(const glm::vec3& newPosition)
    {
        position = newPosition;

        for (auto& vertex : vertices)
        {
            vertex += newPosition; // Desplazar todos los vértices
        }
        // Actualizar el buffer de posiciones
        glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    }

    glm::vec3 ElevationMeshNode::get_position() const
    {
        return position;
    }
    
    void ElevationMeshNode::generate_mesh(const std::string& heightmap_path, float max_height)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(heightmap_path.c_str(), &width, &height, &channels, 1);

        if (!data)
        {
            std::cerr << "Failed to load heightmap: " << heightmap_path << std::endl;
            return;
        }

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float height_value = data[y * width + x] / 255.0f * max_height;
                vertices.emplace_back(0.1f * x, height_value, 0.1f * y);
                uvs.emplace_back(float(x) / (width - 1), float(y) / (height - 1)); // Normalizar entre [0, 1]
            }
        }

        for (int y = 0; y < height - 1; ++y)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                int top_left = y * width + x;
                int top_right = top_left + 1;
                int bottom_left = (y + 1) * width + x;
                int bottom_right = bottom_left + 1;

                indices.push_back(top_left);
                indices.push_back(bottom_left);
                indices.push_back(top_right);

                indices.push_back(top_right);
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
            }
        }

        stbi_image_free(data);

        // Configuración del VAO, VBOs y EBO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Posiciones
        glGenBuffers(1, &vbo_positions);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        // UVs
        GLuint vbo_uvs;
        glGenBuffers(1, &vbo_uvs);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(2);

        // Índices
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Desvincular VAO
        glBindVertexArray(0);
    }


    void ElevationMeshNode::calculate_normals()
    {
        normals.resize(vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            GLuint i0 = indices[i];
            GLuint i1 = indices[i + 1];
            GLuint i2 = indices[i + 2];

            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }

        for (auto& normal : normals)
        {
            normal = glm::normalize(normal);
        }
    }

    void ElevationMeshNode::load_texture(const std::string& texture_path)
    {
        texture_id = SOIL_load_OGL_texture(
            texture_path.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y
        );

        if (!texture_id)
        {
            std::cerr << "Failed to load texture: " << texture_path << std::endl;
        }
    }

    void ElevationMeshNode::draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id)
    {
        glUseProgram(program_id);

        glBindVertexArray(vao);

        glm::mat4 model_view_matrix = view_matrix;
        glUniformMatrix4fv(glGetUniformLocation(program_id, "model_view_matrix"), 1, GL_FALSE, glm::value_ptr(model_view_matrix));
        glUniformMatrix4fv(glGetUniformLocation(program_id, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glActiveTexture(GL_TEXTURE0); // Activa la unidad de textura 0
        glBindTexture(GL_TEXTURE_2D, texture_id); // Enlaza la textura cargada
        glUniform1i(glGetUniformLocation(program_id, "texture_sampler"), 0); // Asigna la textura al sampler
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glUseProgram(0);
    }
}