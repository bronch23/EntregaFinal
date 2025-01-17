#pragma once
#include "SceneNode.hpp"
#include <string>
#include <vector>
#include <glad/glad.h>

namespace udit
{
    class ElevationMeshNode : public SceneNode
    {
    private:
        GLuint vao, vbo_positions, vbo_normals, ebo;
        std::vector<glm::vec3> vertices;
        
        std::vector<glm::vec3> normals;
        std::vector<GLuint> indices;
        GLuint texture_id;
        GLuint shader_program;

        void generate_mesh(const std::string& heightmap_path, float max_height);
        void calculate_normals();

    public:
        ElevationMeshNode(const std::string& heightmap_path, float max_height);
        ~ElevationMeshNode();
        void set_position(const glm::vec3& position);
        void load_texture(const std::string& texture_path);
        std::vector<glm::vec2> uvs;
        void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, GLuint program_id) override;
    };
}
