#pragma once

#include "../modelo/catalogo/caso_clinico.h"
#include "../modelo/catalogo/linea_dialogo.h"
#include "../modelo/mision/estado_mision.h"
#include "../modelo/mision/objetivo_mision.h"

#include <optional>
#include <string>
#include <vector>

namespace veterinaria {

enum class Pantalla { clinica, mision, inicio };

enum class FasePantalla {
    sin_casos,
    llegada,
    por_examinar,
    resultado_examen,
    aceptacion,
    rechazo,
    jugando,
    regresando,
    fallo,
    reintento,
    tratamiento,
    portada
};

enum class Accion {
    examinar,
    aceptar,
    rechazar,
    reintentar,
    avanzar_dialogo,
    alternar_menu,
    iniciar_partida,
    nueva_partida,
    guardar_progreso,
    reiniciar_partida,
    confirmar_reinicio,
    salir_partida
};

struct EntradaCuadro {
    Punto direccion;
    std::optional<Accion> accion;
    bool salir;
};

struct EstadoPantalla {
    Pantalla pantalla;
    FasePantalla fase;
    std::vector<Accion> acciones_disponibles;
    bool menu_abierto;
    float opacidad_fundido;

    // Punteros prestados, nulos si el elemento no se muestra. Validos durante el dibujo.
    const CasoClinico* ficha;
    const LineaDialogo* dialogo;
    const EstadoMision* mision;
    const CantidadSuministro* aviso_recogida;
    std::optional<double> edad_humana_mascota;
    std::optional<MotivoFalloMision> motivo_fallo;
    int saldo;
    std::string mensaje_guardado;
    bool confirmar_reinicio;
    bool salida_solicitada;
};

} // namespace veterinaria
