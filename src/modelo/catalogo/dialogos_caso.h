#pragma once

#include "linea_dialogo.h"

#include <vector>

namespace veterinaria {

struct DialogosCaso {
    std::vector<LineaDialogo> llegada;
    std::vector<LineaDialogo> aceptacion;
    std::vector<LineaDialogo> rechazo;
    std::vector<LineaDialogo> fallo;
    std::vector<LineaDialogo> tratamiento;
};

} // namespace veterinaria
