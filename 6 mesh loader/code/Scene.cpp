#pragma once

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
#include "ElevationNode.hpp"
#include <fstream> // Para guardar en archivos
#include "json.hpp"

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

        skybox_node = std::make_shared<SkyboxNode>("../../../shared/assets/cubemap-", camera_node);
        root_node->add_child(skybox_node);

        light_node = std::make_shared<LightNode>(program_id);
        root_node->add_child(light_node);

        mesh_node = std::make_shared<MeshNode>();
        root_node->add_child(mesh_node);

        elevation_node = std::make_shared<ElevationMeshNode>("../../../shared/assets/height-map.png", 100.0f);
        root_node->add_child(elevation_node);

        initialize_default_scene();


        //load_scene("../../../shared/assets/scene_data.json");


        angle_around_x = angle_delta_x = 0.0;
        angle_around_y = angle_delta_y = 0.0;
        pointer_pressed = false;

        
    }

    void Scene::initialize_default_scene()
    {
        // Leer el archivo JSON
        std::ifstream file("../../../shared/assets/scene_data.json");
        json scene_data;

        if (file.is_open())
        {
            file >> scene_data;
        }
        else
        {
            std::cerr << "Advertencia: No se pudo abrir el archivo scene_data.json, se usar�n valores predeterminados." << std::endl;
        }

        // Malla 1: Fox
        std::string fox_model_path = "../../../shared/assets/Foxx.fbx";
        std::string fox_texture_path = "../../../shared/assets/Fox_BaseColor.png";
        glm::vec3 fox_position = glm::vec3(0.f, -20.f, -255.f);
        float fox_rotation_speed = 1.0f;
        float fox_transparency = 0.9f;

        // Sobrescribir valores desde el JSON si est�n presentes
        if (scene_data.contains("meshes") && scene_data["meshes"].size() > 0)
        {
            const auto& fox_data = scene_data["meshes"][0];
            if (fox_data.contains("model_path")) fox_model_path = fox_data["model_path"];
            if (fox_data.contains("texture_path")) fox_texture_path = fox_data["texture_path"];
            if (fox_data.contains("position"))
            {
                fox_position = glm::vec3(
                    fox_data["position"][0],
                    fox_data["position"][1],
                    fox_data["position"][2]
                );
            }
            if (fox_data.contains("rotation_speed")) fox_rotation_speed = fox_data["rotation_speed"];
            if (fox_data.contains("transparency")) fox_transparency = fox_data["transparency"];
        }

        mesh_node->add_mesh(MeshLoader::load_mesh(fox_model_path,
            glm::rotate(glm::translate(glm::mat4(1.0f), fox_position),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            fox_texture_path));
        mesh_node->meshes.back().rotate_y = true;
        mesh_node->meshes.back().rotation_speed = fox_rotation_speed;
        mesh_node->set_mesh_transparency(mesh_node->meshes.size() - 1, fox_transparency);

        // Malla 2: Pig
        std::string pig_model_path = "../../../shared/assets/Pig.fbx";
        std::string pig_texture_path = "../../../shared/assets/Pig_BaseColor.png";
        glm::vec3 pig_position = glm::vec3(-300.f, -20.f, -400.f);

        if (scene_data.contains("meshes") && scene_data["meshes"].size() > 1)
        {
            const auto& pig_data = scene_data["meshes"][1];
            if (pig_data.contains("model_path")) pig_model_path = pig_data["model_path"];
            if (pig_data.contains("texture_path")) pig_texture_path = pig_data["texture_path"];
            if (pig_data.contains("position"))
            {
                pig_position = glm::vec3(
                    pig_data["position"][0],
                    pig_data["position"][1],
                    pig_data["position"][2]
                );
            }
        }

        mesh_node->add_mesh(MeshLoader::load_mesh(pig_model_path,
            glm::rotate(glm::translate(glm::mat4(1.0f), pig_position),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            pig_texture_path));

        // Malla 3: Wolf
        std::string wolf_model_path = "../../../shared/assets/Wolf.fbx";
        std::string wolf_texture_path = "../../../shared/assets/Wolf_BaseColor.png";
        glm::vec3 wolf_position = glm::vec3(300.f, -150.f, -400.f);

        if (scene_data.contains("meshes") && scene_data["meshes"].size() > 2)
        {
            const auto& wolf_data = scene_data["meshes"][2];
            if (wolf_data.contains("model_path")) wolf_model_path = wolf_data["model_path"];
            if (wolf_data.contains("texture_path")) wolf_texture_path = wolf_data["texture_path"];
            if (wolf_data.contains("position"))
            {
                wolf_position = glm::vec3(
                    wolf_data["position"][0],
                    wolf_data["position"][1],
                    wolf_data["position"][2]
                );
            }
        }

        mesh_node->add_mesh(MeshLoader::load_mesh(wolf_model_path,
            glm::rotate(glm::translate(glm::mat4(1.0f), wolf_position),
                glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
            wolf_texture_path));

        // Nodo de elevaci�n
        glm::vec3 elevation_position = glm::vec3(300.0f, -100.f, -400.0f);
        std::string elevation_texture_path = "../../../shared/assets/uv-checker.png";

        if (scene_data.contains("elevation"))
        {
            const auto& elevation_data = scene_data["elevation"];
            if (elevation_data.contains("position"))
            {
                elevation_position = glm::vec3(
                    elevation_data["position"][0],
                    elevation_data["position"][1],
                    elevation_data["position"][2]
                );
            }
            if (elevation_data.contains("heightmap"))
            {
                elevation_texture_path = elevation_data["heightmap"];
            }
        }

        elevation_node->set_position(elevation_position);
        elevation_node->load_texture(elevation_texture_path);

        for (const auto& light_data : scene_data["lights"])
        {
            glm::vec3 position = {
                light_data["position"][0],
                light_data["position"][1],
                light_data["position"][2]
            };
            glm::vec3 color = {
                light_data["color"][0],
                light_data["color"][1],
                light_data["color"][2]
            };
            float ambient_intensity = light_data["ambient_intensity"];
            float diffuse_intensity = light_data["diffuse_intensity"];

            light_node->set_position(position);
            light_node->set_color(color);
            light_node->set_ambient_intensity(ambient_intensity);
            light_node->set_diffuse_intensity(diffuse_intensity);
        }

        if (scene_data["postprocessing"].contains("effect_intensity"))
        {
            postprocess_effect_intensity = scene_data["postprocessing"]["effect_intensity"];
        }

        if (scene_data["postprocessing"].contains("color_adjustment"))
        {
            postprocess_color_adjustment = {
                scene_data["postprocessing"]["color_adjustment"][0],
                scene_data["postprocessing"]["color_adjustment"][1],
                scene_data["postprocessing"]["color_adjustment"][2]
            };
        }
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
        // L�gica existente para la c�mara
        /*camera.apply_rotation();*/

        // Actualizaci�n del nodo ra�z
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

        GLuint effect_intensity_loc = glGetUniformLocation(screen_shader_program, "effect_intensity");
        GLuint color_adjustment_loc = glGetUniformLocation(screen_shader_program, "color_adjustment");

        // Asignar valores din�micos
        glUniform1f(effect_intensity_loc, postprocess_effect_intensity);
        glUniform3fv(color_adjustment_loc, 1, glm::value_ptr(postprocess_color_adjustment));

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
    uniform float effect_intensity;
    uniform vec3 color_adjustment;

    void main()
    {
        vec3 color = texture(screen_texture, uv).rgb;

        // Ajuste de color basado en los par�metros din�micos
        color.r += effect_intensity * color_adjustment.r;
        color.g += effect_intensity * color_adjustment.g;
        color.b *= effect_intensity * color_adjustment.b;

        frag_color = vec4(color, 1.0);
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


    void Scene::save_scene(const std::string& file_path)
    {
        json scene_data;

        // Guardar datos de la c�mara
        scene_data["camera"] = {
            {"position", {camera_node->position.x, camera_node->position.y, camera_node->position.z}},
            {"rotation", {camera_node->rotation.x, camera_node->rotation.y, camera_node->rotation.z}},
            {"aspect_ratio", camera_node->get_ratio()},
            {"fov", camera_node->get_fov()}
        };

        if (mesh_node) // Usar la referencia directa al MeshNode
        {
            for (const auto& mesh : mesh_node->meshes) // Recorrer todas las mallas
            {
                glm::vec3 position = glm::vec3(mesh.model_matrix[3]); // Extraer posici�n de la matriz de modelo
                scene_data["meshes"].push_back({
                    {"model_path", mesh.model_path},       // Ruta del modelo
                    {"texture_path", mesh.texture_path},   // Ruta de la textura
                    {"position", {position.x, position.y, position.z}}, // Posici�n
                    {"rotation_speed", mesh.rotation_speed}, // Velocidad de rotaci�n
                    {"transparency", mesh.transparency}     // Transparencia
                    });
            }
        }

        if (elevation_node)
        {
            glm::vec3 position = elevation_node->get_position();
            scene_data["elevation"] = {
                {"heightmap", "../../../shared/assets/height-map.png"}, // Ruta del heightmap
                {"position", {position.x, position.y, position.z}},     // Posici�n
                {"scale", 100.0f}                                       // Escala
            };
        }

        if (light_node) 
        {
            glm::vec3 position = light_node->get_position();
            glm::vec3 color = light_node->get_color();
            float ambient_intensity = light_node->get_ambient_intensity();
            float diffuse_intensity = light_node->get_diffuse_intensity();

            scene_data["lights"].push_back({
                {"type", "directional"},
                {"position", {position.x, position.y, position.z}},
                {"color", {color.r, color.g, color.b}},
                {"ambient_intensity", ambient_intensity},
                {"diffuse_intensity", diffuse_intensity}
                });
        }

        if (skybox_node) 
        {
            std::string texture_base_path = skybox_node->get_texture_base_path();

            scene_data["skybox"] = {
                {"texture_base_path", texture_base_path}
            };
        }

        scene_data["postprocessing"] = {
                {"effect_intensity", postprocess_effect_intensity},
                {"color_adjustment", {
                postprocess_color_adjustment.r,
                postprocess_color_adjustment.g,
                postprocess_color_adjustment.b
                }}
        };

        // Escribir el JSON en un archivo
        std::ofstream file(file_path);
        if (file.is_open())
        {
            file << scene_data.dump(4); 
            file.close();
            std::cout << "Escena guardada en " << file_path << std::endl;
        }
        else
        {
            std::cerr << "Error al guardar la escena en " << file_path << std::endl;
        }
    }

//    void Scene::load_scene(const std::string& file_path)
//    {
//        std::ifstream file(file_path);
//
//        json scene_data;
//        file >> scene_data;
//
//        // Llamar a m�todos para reconstruir cada parte de la escena
//        load_camera(scene_data["camera"]);
//        load_meshes(scene_data["meshes"]);
//        load_elevation(scene_data["elevation"]);
//        load_lights(scene_data["lights"]);
//        load_skybox(scene_data["skybox"]);
//        load_postprocessing(scene_data["postprocessing"]);
//
//        std::cout << "Escena cargada desde " << file_path << std::endl;
//    }
//
//    void Scene::load_camera(const json& camera_data)
//    {
//        glm::vec3 position = {
//            camera_data["position"][0],
//            camera_data["position"][1],
//            camera_data["position"][2]
//        };
//        glm::vec3 rotation = {
//            camera_data["rotation"][0],
//            camera_data["rotation"][1],
//            camera_data["rotation"][2]
//        };
//        float aspect_ratio = camera_data["aspect_ratio"];
//        float fov = camera_data["fov"];
//
//        camera_node->set_position(position.x, position.y, position.z);
//        camera_node->set_ratio(aspect_ratio);
//        camera_node->set_fov(fov);
//    }
//
//    void Scene::load_meshes(const json& meshes_data)
//    {
//        for (const auto& mesh_data : meshes_data)
//        {
//            std::string model_path = mesh_data["model_path"];
//            std::string texture_path = mesh_data["texture_path"];
//            glm::vec3 position = {
//                mesh_data["position"][0],
//                mesh_data["position"][1],
//                mesh_data["position"][2]
//            };
//
//            auto mesh = MeshLoader::load_mesh(
//                model_path,
//                glm::rotate(glm::translate(glm::mat4(1.0f), position),
//                    glm::radians(-90.0f), glm::vec3(1.f, 0.f, 0.f)),
//                texture_path
//            );
//
//            mesh_node->add_mesh(mesh);
//        }
//    }
//
//    void Scene::load_elevation(const json& elevation_data)
//    {
//        std::string heightmap = elevation_data["heightmap"];
//        glm::vec3 position = {
//            elevation_data["position"][0],
//            elevation_data["position"][1],
//            elevation_data["position"][2]
//        };
//        float scale = elevation_data["scale"];
//
//        elevation_node = std::make_shared<ElevationMeshNode>(heightmap, scale);
//        elevation_node->set_position(position);
//        root_node->add_child(elevation_node);
//    }
//
//    void Scene::load_lights(const json& lights_data)
//    {
//        for (const auto& light_data : lights_data)
//        {
//            glm::vec3 position = {
//                light_data["position"][0],
//                light_data["position"][1],
//                light_data["position"][2]
//            };
//            glm::vec3 color = {
//                light_data["color"][0],
//                light_data["color"][1],
//                light_data["color"][2]
//            };
//            float ambient_intensity = light_data["ambient_intensity"];
//            float diffuse_intensity = light_data["diffuse_intensity"];
//
//            light_node->set_position(position);
//            light_node->set_color(color);
//            light_node->set_ambient_intensity(ambient_intensity);
//            light_node->set_diffuse_intensity(diffuse_intensity);
//        }
//    }
//
//    void Scene::load_skybox(const json& skybox_data)
//    {
//        std::string texture_base_path = skybox_data["texture_base_path"];
//        skybox_node = std::make_shared<SkyboxNode>(texture_base_path, camera_node);
//        root_node->add_child(skybox_node);
//    }
//
//    void Scene::load_postprocessing(const json& postprocessing_data)
//    {
//        if (postprocessing_data.contains("effect_intensity"))
//        {
//            postprocess_effect_intensity = postprocessing_data["effect_intensity"];
//        }
//
//        if (postprocessing_data.contains("color_adjustment"))
//        {
//            postprocess_color_adjustment = {
//                postprocessing_data["color_adjustment"][0],
//                postprocessing_data["color_adjustment"][1],
//                postprocessing_data["color_adjustment"][2]
//            };
//        }
//    }
}



