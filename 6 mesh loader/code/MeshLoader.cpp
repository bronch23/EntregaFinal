#include "MeshLoader.hpp"
#include <SOIL2.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace udit
{
    MeshNode::Mesh MeshLoader::load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path)
    {
        MeshNode::Mesh mesh;
        mesh.model_matrix = model_matrix;

        // Asignar rutas
        mesh.model_path = mesh_file_path;
        mesh.texture_path = texture_path;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(mesh_file_path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

        if (scene && scene->mNumMeshes > 0)
        {
            const aiMesh* ai_mesh = scene->mMeshes[0];

            glGenBuffers(4, mesh.vbo_ids);
            glGenVertexArrays(1, &mesh.vao_id);
            glBindVertexArray(mesh.vao_id);

            // Configurar vértices
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_ids[0]);
            glBufferData(GL_ARRAY_BUFFER, ai_mesh->mNumVertices * sizeof(aiVector3D), ai_mesh->mVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Configurar normales
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_ids[1]);
            glBufferData(GL_ARRAY_BUFFER, ai_mesh->mNumVertices * sizeof(aiVector3D), ai_mesh->mNormals, GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Configurar coordenadas UV
            std::vector<glm::vec2> texture_coords(ai_mesh->mNumVertices);
            for (unsigned i = 0; i < ai_mesh->mNumVertices; ++i)
            {
                if (ai_mesh->mTextureCoords[0])
                {
                    texture_coords[i] = glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_ids[2]);
            glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(glm::vec2), texture_coords.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

            // Configurar índices
            std::vector<GLushort> indices;
            for (unsigned i = 0; i < ai_mesh->mNumFaces; ++i)
            {
                const aiFace& face = ai_mesh->mFaces[i];
                indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
            }
            mesh.number_of_indices = static_cast<GLsizei>(indices.size());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo_ids[3]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

            // Cargar textura
            mesh.texture_id = SOIL_load_OGL_texture(texture_path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        }
        else
        {
            std::cerr << "Failed to load mesh: " << mesh_file_path << std::endl;
        }

        return mesh;
    }

}
