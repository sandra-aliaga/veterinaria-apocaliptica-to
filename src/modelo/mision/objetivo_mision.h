#pragma once

#include "../catalogo/cantidad_suministro.h"

#include <optional>
#include <string>
#include <vector>

namespace veterinaria {

struct PropuestaMision {
    std::string id_caso;
    std::vector<CantidadSuministro> requeridos;
    int limite_segundos = 300;
};

enum class DesenlaceMision { completada, fallida };
enum class MotivoFalloMision { sin_vida, tiempo_agotado };

struct ResultadoMision {
    std::string id_caso;
    DesenlaceMision desenlace;
    std::vector<CantidadSuministro> recogidos;
    std::optional<MotivoFalloMision> motivo_fallo;
};

} // namespace veterinaria
