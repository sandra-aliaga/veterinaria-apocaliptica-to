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
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

constexpr char directorio_arte[] = "arte";

void usar_recursos_del_ejecutable() {
    namespace ajustes = veterinaria::configuracion_juego;
    const auto directorio = std::filesystem::u8path(GetApplicationDirectory());

    if (std::filesystem::is_directory(directorio / ajustes::directorio_casos) &&
        std::filesystem::is_directory(directorio / directorio_arte)) {
        std::error_code error;
        std::filesystem::current_path(directorio, error);

        if (error) {
            throw std::runtime_error("No se pudo abrir la carpeta de recursos del juego.");
        }
    }
}

} // namespace

int main() try {
    namespace ajustes = veterinaria::configuracion_juego;

    usar_recursos_del_ejecutable();

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
    auto arma = std::make_unique<veterinaria::ArmaBase>(ajustes::danio_arma, ajustes::recarga_arma,
                                                        ajustes::alcance_arma);
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
