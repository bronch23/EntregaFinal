
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
#include "MeshLoader.hpp"
#include "LightNode.hpp"


namespace udit
{

    Scene::Scene(int width, int height) : angle(0)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        framebuffer.initialize(width, height);
        initialize_screen_quad();
        screen_shader_program = compile_screen_shaders();

        root_node = std::make_shared<SceneNode>();
        program_id = compile_shaders();
        glUseProgram(program_id);

        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");
        normal_matrix_id = glGetUniformLocation(program_id, "normal_matrix");

        resize(width, height);

        camera_node = std::make_shared<CameraNode>(float(width) / height);
        root_node->add_child(camera_node);

        auto skybox_node = std::make_shared<SkyboxNode>("../../../shared/assets/cubemap-", camera_node);
        root_node->add_child(skybox_node);

        // Crear el nodo de luz y agregarlo al grafo
        auto light_node = std::make_shared<LightNode>(program_id);
        root_node->add_child(light_node);

        auto mesh_node = std::make_shared<MeshNode>();
        root_node->add_child(mesh_node);

        // Cargar y asignar mallas al nodo
        mesh_node->add_mesh(MeshLoader::load_mesh("../../../shared/assets/Foxx.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -20.f, -255.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Fox_BaseColor.png"));
        mesh_node->meshes.back().rotate_y = true; // Activa rotación para esta malla
        mesh_node->meshes.back().rotation_speed = 1.f; // Ajusta velocidad de rotación
        // Establecer transparencia para el último mesh
        mesh_node->set_mesh_transparency(mesh_node->meshes.size() - 1, 0.9f);

        mesh_node->add_mesh(MeshLoader::load_mesh("../../../shared/assets/Pig.fbx",
            glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-300.f, -20.f, -400.f)),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            "../../../shared/assets/Pig_BaseColor.png"));

        mesh_node->add_mesh(MeshLoader::load_mesh("../../../shared/assets/Wolf.fbx",
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
        float delta_time = 0.016f;
        // Lógica existente para la cámara
        /*camera.apply_rotation();*/

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
        // Renderizar la escena al framebuffer
        framebuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = camera_node->get_transform_matrix_inverse();
        glm::mat4 projection_matrix = camera_node->get_projection_matrix();

        glUseProgram(program_id);
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        root_node->draw(view_matrix, projection_matrix, program_id);

        glUseProgram(0);
        framebuffer.unbind();

        // Renderizar el contenido del framebuffer en pantalla
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(screen_shader_program);

        glBindVertexArray(screen_quad_vao);
        glBindTexture(GL_TEXTURE_2D, framebuffer.get_texture());
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindVertexArray(0);
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
            float delta_x = 0.025f * float(last_pointer_x - pointer_x) / float(width);
            float delta_y = 0.025f * float(last_pointer_y - pointer_y) / float(height);
            camera_node->add_rotation_delta(delta_x, delta_y);
        }
    }

    void Scene::on_click(int pointer_x, int pointer_y, bool down)
    {
        if ((pointer_pressed = down))
        {
            last_pointer_x = pointer_x;
            last_pointer_y = pointer_y;
        }
    }
    void Scene::initialize_screen_quad()
    {
        GLuint quad_vao, quad_vbo;
        float quad_vertices[] = {
            // Posiciones   // Coordenadas UV
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
        };

        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);

        glBindVertexArray(quad_vao);

        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);

        screen_quad_vao = quad_vao; // Guardar el VAO para renderizar el framebuffer
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
        uniform float transparency; // Nuevo atributo de transparencia
        out vec4 fragment_color;

        void main(){
        
            vec4 tex_color = texture(texture_sampler, texture_uv);
            fragment_color = vec4(front_color, 1.0) * tex_color;
            fragment_color.a *= transparency; // Aplicar transparencia
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

    GLuint Scene::compile_screen_shaders()
    {
        const char* vertex_shader_code = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    layout(location = 1) in vec2 tex_coords;

    out vec2 uv;

    void main()
    {
        uv = tex_coords;
        gl_Position = vec4(position, 0.0, 1.0);
    }
    )";

        const char* fragment_shader_code = R"(
    #version 330 core
    in vec2 uv;
    out vec4 frag_color;

    uniform sampler2D screen_texture;

    void main()
    {
        frag_color = texture(screen_texture, uv);
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



