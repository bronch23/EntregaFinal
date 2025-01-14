
#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include <string>
#include <glm.hpp> // Biblioteca para matem�ticas 3D
#include <glad/glad.h>

namespace udit
{
    class Scene
    {
    public:
        Scene(int width, int height);
        ~Scene();

        void update();
        void render();
        void resize(int width, int height);

    private:
        struct Mesh
        {
            GLuint vao_id; // Identificador del Vertex Array Object
            GLuint vbo_ids[3]; // Buffers para v�rtices, colores e �ndices
            GLsizei number_of_indices; // Cantidad de �ndices
            glm::mat4 model_matrix; // Matriz de transformaci�n
        };

        std::vector<Mesh> meshes; // Contenedor para m�ltiples mallas

        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;

        float angle;

        void load_mesh(const std::string& mesh_file_path, const glm::mat4& model_matrix);
        GLuint compile_shaders();
        glm::vec3 random_color();
    };
}

#endif

