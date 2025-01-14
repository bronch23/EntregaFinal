
#include "Scene.hpp"
#include <iostream>
#include <SOIL2.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace udit
{

    Scene::Scene(int width, int height) : angle(0)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        program_id = compile_shaders();
        glUseProgram(program_id);

        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");

        resize(width, height);

        // Cargar mallas con texturas
        load_mesh("../../../shared/assets/Foxx.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -20.f, -255.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Fox_BaseColor.png");

        load_mesh("../../../shared/assets/Pig.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-300.f, -20.f, -400.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Pig_BaseColor.png");

        load_mesh("../../../shared/assets/Wolf.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(300.f, -20.f, -400.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Wolf_BaseColor.png");
    }

    Scene::~Scene()
    {
        for (const auto& mesh : meshes)
        {
            glDeleteVertexArrays(1, &mesh.vao_id);
            glDeleteBuffers(3, mesh.vbo_ids);
            glDeleteTextures(1, &mesh.texture_id);
        }
    }

    void Scene::load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(mesh_file_path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

        if (scene && scene->mNumMeshes > 0)
        {
            const aiMesh* ai_mesh = scene->mMeshes[0];
            Mesh mesh;
            mesh.model_matrix = model_matrix;

            glGenBuffers(3, mesh.vbo_ids);
            glGenVertexArrays(1, &mesh.vao_id);
            glBindVertexArray(mesh.vao_id);

            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_ids[0]);
            glBufferData(GL_ARRAY_BUFFER, ai_mesh->mNumVertices * sizeof(aiVector3D), ai_mesh->mVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            std::vector<glm::vec2> texture_coords(ai_mesh->mNumVertices);
            for (unsigned i = 0; i < ai_mesh->mNumVertices; ++i)
            {
                if (ai_mesh->mTextureCoords[0])
                {
                    texture_coords[i] = glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_ids[1]);
            glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(glm::vec2), texture_coords.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

            std::vector<GLushort> indices;
            for (unsigned i = 0; i < ai_mesh->mNumFaces; ++i)
            {
                const aiFace& face = ai_mesh->mFaces[i];
                indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
            }
            mesh.number_of_indices = indices.size();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo_ids[2]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

            mesh.texture_id = create_texture_2d(texture_path);
            meshes.push_back(mesh);
        }
        else
        {
            std::cerr << "Failed to load mesh: " << mesh_file_path << std::endl;
        }
    }

    GLuint Scene::create_texture_2d(const std::string& texture_path)
    {
        GLuint texture_id = SOIL_load_OGL_texture(
            texture_path.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y
        );

        if (!texture_id)
        {
            std::cerr << "Failed to load texture: " << texture_path << std::endl;
        }

        return texture_id;
    }

    void Scene::update()
    {
        angle += 0.01f;
    }

    void Scene::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& mesh : meshes)
        {
            glm::mat4 model_view_matrix = glm::rotate(mesh.model_matrix, angle, glm::vec3(0.f, 0.f, 1.f));
            glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));

            glBindTexture(GL_TEXTURE_2D, mesh.texture_id);

            glBindVertexArray(mesh.vao_id);
            glDrawElements(GL_TRIANGLES, mesh.number_of_indices, GL_UNSIGNED_SHORT, 0);
        }
    }

    void Scene::resize(int width, int height)
    {
        glm::mat4 projection_matrix = glm::perspective(20.f, float(width) / height, 1.f, 5000.f);
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glViewport(0, 0, width, height);
    }

    GLuint Scene::compile_shaders()
    {
        const char* vertex_shader_code = R"(
        #version 330
        uniform mat4 model_view_matrix;
        uniform mat4 projection_matrix;
        layout (location = 0) in vec3 vertex_coordinates;
        layout (location = 1) in vec2 vertex_texture_uv;
        out vec2 texture_uv;
        void main() {
            gl_Position = projection_matrix * model_view_matrix * vec4(vertex_coordinates, 1.0);
            texture_uv = vertex_texture_uv;
        }
    )";

        const char* fragment_shader_code = R"(
        #version 330
        uniform sampler2D texture_sampler;
        in vec2 texture_uv;
        out vec4 fragment_color;
        void main() {
            fragment_color = texture(texture_sampler, texture_uv);
        }
    )";

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
        glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);

        glCompileShader(vertex_shader);
        glCompileShader(fragment_shader);

        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, vertex_shader);
        glAttachShader(program_id, fragment_shader);

        glLinkProgram(program_id);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return program_id;
    }

}



