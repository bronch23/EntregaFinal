
#include "Scene.hpp"
#include <iostream>
#include <SOIL2.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "MeshNode.hpp"
#include "SkyboxNode.hpp"


namespace udit
{

    Scene::Scene(int width, int height) : angle(0)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        root_node = std::make_shared<SceneNode>();

        program_id = compile_shaders();
        glUseProgram(program_id);

        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");
        normal_matrix_id = glGetUniformLocation(program_id, "normal_matrix");

        resize(width, height);

        GLint light_position = glGetUniformLocation(program_id, "light.position");
        GLint light_color = glGetUniformLocation(program_id, "light.color");
        GLint ambient_intensity = glGetUniformLocation(program_id, "ambient_intensity");
        GLint diffuse_intensity = glGetUniformLocation(program_id, "diffuse_intensity");
        glUniform4f(light_position, 10.0f, 10.0f, 10.0f, 1.0f);
        glUniform3f(light_color, 1.0f, 1.0f, 1.0f);
        glUniform1f(ambient_intensity, 0.2f);
        glUniform1f(diffuse_intensity, 0.8f);

        auto skybox_node = std::make_shared<SkyboxNode>("../../../shared/assets/cubemap-", camera);
        root_node->add_child(skybox_node);

        auto mesh_node = std::make_shared<MeshNode>();
        root_node->add_child(mesh_node);

        // Cargar y asignar mallas al nodo
        mesh_node->add_mesh(create_mesh("../../../shared/assets/Foxx.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -20.f, -255.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Fox_BaseColor.png"));
        mesh_node->meshes.back().rotate_y = true; // Activa rotación para esta malla
        mesh_node->meshes.back().rotation_speed = 1.f; // Ajusta velocidad de rotación

        mesh_node->add_mesh(create_mesh("../../../shared/assets/Pig.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-300.f, -20.f, -400.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Pig_BaseColor.png"));

        mesh_node->add_mesh(create_mesh("../../../shared/assets/Wolf.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(300.f, -150.f, -400.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Wolf_BaseColor.png"));

        angle_around_x = angle_delta_x = 0.0;
        angle_around_y = angle_delta_y = 0.0;
        pointer_pressed = false;
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

    MeshNode::Mesh Scene::create_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix, const std::string& texture_path)
    {
        MeshNode::Mesh mesh;
        mesh.model_matrix = model_matrix;

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
            mesh.texture_id = create_texture_2d(texture_path);
        }
        else
        {
            std::cerr << "Failed to load mesh: " << mesh_file_path << std::endl;
        }

        return mesh;
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
        // Lógica existente para la cámara
        angle += 0.01f;
        angle_around_x += angle_delta_x;
        angle_around_y += angle_delta_y;
        float delta_time = 0.016f;
        if (angle_around_x < -1.5)
        {
            angle_around_x = -1.5;
        }
        else if (angle_around_x > +1.5)
        {
            angle_around_x = +1.5;
        }

        glm::mat4 camera_rotation(1);
        camera_rotation = glm::rotate(camera_rotation, angle_around_y, glm::vec3(0.f, 1.f, 0.f));
        camera_rotation = glm::rotate(camera_rotation, angle_around_x, glm::vec3(1.f, 0.f, 0.f));

        camera.set_target(0, 0, -1);
        camera.rotate(camera_rotation);

        // Actualización del nodo raíz
        root_node->update();
        for (const auto& child : root_node->get_children())
        {
            auto mesh_node = std::dynamic_pointer_cast<MeshNode>(child);
            if (mesh_node)
            {
                mesh_node->update_meshes(delta_time);
            }
        }
    }

    void Scene::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Obtener las matrices de la cámara
        glm::mat4 view_matrix = camera.get_transform_matrix_inverse();
        glm::mat4 projection_matrix = camera.get_projection_matrix();

        // Activar el programa de shaders
        glUseProgram(program_id);

        // Configurar la matriz de proyección en el shader
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        // Dibujar el grafo de escena
        root_node->draw(view_matrix, projection_matrix, program_id);

        // Desactivar el programa de shaders
        glUseProgram(0);
    }



    void Scene::resize(int new_width, int new_height)
    {
        glm::mat4 projection_matrix = glm::perspective(20.f, float(new_width) / new_height, 1.f, 5000.f);
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glViewport(0, 0, new_width, new_height);

        width = new_width;
        height = new_height;
        camera.set_ratio(float(new_width) / new_height);
    }
    void Scene::on_drag(int pointer_x, int pointer_y)
    {
        if (pointer_pressed)
        {
            angle_delta_x = 0.025f * float(last_pointer_y - pointer_y) / float(height);
            angle_delta_y = 0.025f * float(last_pointer_x - pointer_x) / float(width);
        }
    }

    void Scene::on_click(int pointer_x, int pointer_y, bool down)
    {
        if ((pointer_pressed = down) == true)
        {
            last_pointer_x = pointer_x;
            last_pointer_y = pointer_y;
        }
        else
        {
            angle_delta_x = angle_delta_y = 0.0;
        }
    }
    GLuint Scene::compile_shaders()
    {
        const char* vertex_shader_code = R"(
    #version 330
    struct Light {
        vec4 position;
        vec3 color;
    };
    uniform Light light;
    uniform float ambient_intensity;
    uniform float diffuse_intensity;
    uniform mat4 model_view_matrix;
    uniform mat4 projection_matrix;
    uniform mat4 normal_matrix;
    layout (location = 0) in vec3 vertex_coordinates;
    layout (location = 1) in vec3 vertex_normal;
    layout (location = 2) in vec2 vertex_texture_uv;
    out vec2 texture_uv;
    out vec3 front_color;
    void main() {
        vec4 position = model_view_matrix * vec4(vertex_coordinates, 1.0);
        vec4 normal = normal_matrix * vec4(vertex_normal, 0.0);
        vec4 light_dir = light.position - position;
        float intensity = diffuse_intensity * max(dot(normalize(normal.xyz), normalize(light_dir.xyz)), 0.0);
        front_color = (ambient_intensity + intensity) * light.color;
        texture_uv = vertex_texture_uv;
        gl_Position = projection_matrix * position;
    }
    )";

        const char* fragment_shader_code = R"(
    #version 330
    in vec3 front_color;
    in vec2 texture_uv;
    uniform sampler2D texture_sampler;
    out vec4 fragment_color;
    void main() {
        vec4 tex_color = texture(texture_sampler, texture_uv);
        fragment_color = vec4(front_color, 1.0) * tex_color;
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



