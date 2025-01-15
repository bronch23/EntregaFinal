
// Este código es de dominio público
// angel.rodriguez@udit.es

#include "Scene.hpp"
#include <Window.hpp>

using udit::Scene;
using udit::Window;

int main (int , char * [])
{
    constexpr unsigned viewport_width  = 1024;
    constexpr unsigned viewport_height =  576;

    Window window
    (
        "OpenGL example",
        Window::Position::CENTERED,
        Window::Position::CENTERED,
        viewport_width,
        viewport_height, 
        { 3, 3 }
    );
    Scene scene(viewport_width, viewport_height);
    bool exit = false;
    
    do
    {
        // Se procesan los eventos acumulados:

        SDL_Event event;

        while (SDL_PollEvent (&event) > 0)
        {
            switch (event.type)
            {
            case SDL_MOUSEBUTTONDOWN:
                scene.on_click(event.button.x, event.button.y, true);
                break;

            case SDL_MOUSEBUTTONUP:
                scene.on_click(event.button.x, event.button.y, false);
                break;

            case SDL_MOUSEMOTION:
                scene.on_drag(event.motion.x, event.motion.y);
                break;

            case SDL_QUIT:
                exit = true;
                break;
            }
        }

        // Se actualiza la escena:

        scene.update ();

        // Se redibuja la escena:

        scene.render ();

        // Se actualiza el contenido de la ventana:

        window.swap_buffers ();
    }
    while (not exit);

    SDL_Quit ();

    return 0;
}
