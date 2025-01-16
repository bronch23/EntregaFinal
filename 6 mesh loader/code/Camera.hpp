
#ifndef CAMERA_HEADER
#define CAMERA_HEADER

    #include <glm.hpp>                          // vec3, vec4, ivec4, mat4
    #include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
    #include <gtc/type_ptr.hpp>                 // value_ptr

namespace udit
{

    class Camera
    {
        using Point = glm::vec4;
        using Vector = glm::vec4;
        using Matrix44 = glm::mat4;

    private:
        float    fov;
        float    near_z;
        float    far_z;
        float    ratio;

        Point    location;
        Point    target;

        Matrix44 projection_matrix;

        // Variables para manejar rotación
        float angle_around_x = 0.0f;
        float angle_around_y = 0.0f;
        float angle_delta_x = 0.0f;
        float angle_delta_y = 0.0f;

    public:
        Camera(float ratio = 1.f)
        {
            reset(60.f, 0.1f, 1000.f, ratio);
        }

        Camera(float fov_degrees, float near_z, float far_z, float ratio)
        {
            reset(fov_degrees, near_z, far_z, ratio);
        }

        float get_fov() const { return fov; }
        float get_near_z() const { return near_z; }
        float get_far_z() const { return far_z; }
        float get_ratio() const { return ratio; }

        const Point& get_location() const { return location; }
        const Point& get_target() const { return target; }

        void set_fov(float new_fov) { fov = new_fov; calculate_projection_matrix(); }
        void set_near_z(float new_near_z) { near_z = new_near_z; calculate_projection_matrix(); }
        void set_far_z(float new_far_z) { far_z = new_far_z; calculate_projection_matrix(); }
        void set_ratio(float new_ratio) { ratio = new_ratio; calculate_projection_matrix(); }

        void set_location(float x, float y, float z) { location = Point(x, y, z, 1.0f); }
        void set_target(float x, float y, float z) { target = Point(x, y, z, 1.0f); }

        void reset(float fov_degrees, float near_z, float far_z, float ratio)
        {
            set_fov(fov_degrees);
            set_near_z(near_z);
            set_far_z(far_z);
            set_ratio(ratio);
            set_location(0.f, 0.f, 0.f);
            set_target(0.f, 0.f, -1.f);
            angle_around_x = angle_around_y = 0.0f;
            calculate_projection_matrix();
        }

        void move(const glm::vec3& translation)
        {
            location += glm::vec4(translation, 1.f);
            target += glm::vec4(translation, 1.f);
        }

        void rotate(const glm::mat4& rotation)
        {
            target = location + rotation * (target - location);
        }

        // Métodos nuevos para manejar ángulos
        void add_rotation_delta(float delta_x, float delta_y)
        {
            // Invertimos los ejes para que respondan correctamente
            angle_delta_y += delta_x; // Rotación alrededor del eje Y (horizontalmente)
            angle_delta_x += delta_y; // Rotación alrededor del eje X (verticalmente)
        }

        void apply_rotation()
        {
            angle_around_x += angle_delta_x;
            angle_around_y += angle_delta_y;

            // Limitar el rango del ángulo alrededor del eje X
            if (angle_around_x < -1.5f) angle_around_x = -1.5f;
            if (angle_around_x > 1.5f) angle_around_x = 1.5f;

            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle_around_y, glm::vec3(0.f, 1.f, 0.f)); // Rotación alrededor de Y
            rotation = glm::rotate(rotation, angle_around_x, glm::vec3(1.f, 0.f, 0.f));                  // Rotación alrededor de X
            target = location + rotation * glm::vec4(0.f, 0.f, -1.f, 0.f);

            // Resetear deltas después de aplicar la rotación
            angle_delta_x = 0.0f;
            angle_delta_y = 0.0f;
        }

        const glm::mat4& get_projection_matrix() const
        {
            return projection_matrix;
        }

        glm::mat4 get_transform_matrix_inverse() const
        {
            return glm::lookAt(
                glm::vec3(location[0], location[1], location[2]),
                glm::vec3(target[0], target[1], target[2]),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }

    private:
        void calculate_projection_matrix()
        {
            projection_matrix = glm::perspective(glm::radians(fov), ratio, near_z, far_z);
        }
    };

}

#endif
