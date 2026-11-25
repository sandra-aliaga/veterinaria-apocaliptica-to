#include "prueba_jugable/controlador_prueba.h"
#include "vista/vista_raylib.h"

#include <raylib.h>

#include <cstdlib>
#include <exception>
#include <iostream>

int main() try {
    veterinaria::ControladorPrueba controlador;
    auto vista = veterinaria::crear_vista_raylib();
    vista->dibujar(controlador.estado_pantalla());

    while (true) {
        const auto entrada = vista->leer_entrada();
        controlador.actualizar(entrada, GetFrameTime());
        const auto estado = controlador.estado_pantalla();

        if (estado.salida_solicitada) {
            break;
        }
        vista->dibujar(estado);
    }
    return EXIT_SUCCESS;
}

catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return EXIT_FAILURE;
}
