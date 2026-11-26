#include "controlador/controlador_juego.h"
#include "modelo/catalogo/catalogo_casos_json.h"
#include "modelo/catalogo/mascota/fabrica_mascotas_base.h"
#include "modelo/clinica/consultorio_clinica.h"
#include "modelo/mision/combate/arma/arma_base.h"
#include "modelo/mision/combate/combate_base.h"
#include "modelo/mision/combate/enemigo/fabrica_enemigos_base.h"
#include "modelo/mision/mapa/planificador_campo_abierto.h"
#include "modelo/mision/mision_base.h"
#include "vista/vista_raylib.h"

#include <raylib.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

int main() try {
    namespace ajustes = veterinaria::configuracion_juego;

    const veterinaria::FabricaMascotasBase fabrica_mascotas;
    const veterinaria::CatalogoCasosJson catalogo(fabrica_mascotas);
    auto carga = catalogo.cargar(ajustes::directorio_casos);

    for (const auto& error : carga.errores) {
        std::cerr << error.archivo << ": " << error.detalle << '\n';
    }

    if (carga.casos.empty()) {
        std::cerr << "No hay fichas clínicas para iniciar la partida.\n";
        return EXIT_FAILURE;
    }

    veterinaria::ConsultorioClinica consultorio;
    veterinaria::PlanificadorCampoAbierto mapa(ajustes::limites_mision);
    auto fabrica_enemigos = std::make_unique<veterinaria::FabricaEnemigosBase>();
    auto arma = std::make_unique<veterinaria::ArmaBase>(ajustes::danio_arma, ajustes::recarga_arma);
    veterinaria::CombateBase combate(std::move(fabrica_enemigos), std::move(arma));
    veterinaria::MisionBase mision(mapa, combate, ajustes::radio_recogida);
    veterinaria::ControladorJuego controlador(consultorio, mision, fabrica_mascotas,
                                              std::move(carga.casos));
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
