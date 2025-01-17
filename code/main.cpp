// Proyecto: Entrega Programación Gráfica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripción: Implementación de conceptos básicos de programación de gráficos

#include "Scene.hpp"
#include <Window.hpp>

using udit::Scene;
using udit::Window;

bool handle_events(Scene& scene)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            return false; // Salir del programa
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            // Movimiento del ratón para controlar la cámara
            scene.on_drag(event.motion.x, event.motion.y);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
        {
            // Click del ratón
            scene.on_click(event.button.x, event.button.y, event.type == SDL_MOUSEBUTTONDOWN);
        }
    }
    return true;
}

// Función para actualizar la escena
void update_scene(Scene& scene)
{
    scene.update();
}

// Función para renderizar la escena
void render_scene(Scene& scene, Window& window)
{
    scene.render();
    window.swap_buffers();
}

int main(int, char * [])
{
    constexpr unsigned viewport_width = 1024;
    constexpr unsigned viewport_height = 576;

    // Crear la ventana
    Window window(
        "OpenGL Scene",
        Window::Position::CENTERED,
        Window::Position::CENTERED,
        viewport_width,
        viewport_height,
        { 3, 3 } // Versión de OpenGL
    );

    // Crear la escena
    Scene scene(viewport_width, viewport_height);

    //scene.load_scene("scene_data.json");



    // Bucle principal
    bool running = true;
    while (running)
    {
        running = handle_events(scene);
        update_scene(scene);
        render_scene(scene, window);
    }
    //Guardar Escena
    scene.save_scene("../../../shared/assets/scene_data.json");
    SDL_Quit();
    return 0;
}
