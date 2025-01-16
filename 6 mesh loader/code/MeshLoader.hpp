#pragma once
#include <string>
#include <glm.hpp>
#include "MeshNode.hpp"

namespace udit
{
    class MeshLoader
    {
    public:
        static MeshNode::Mesh load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path);
    };
}

