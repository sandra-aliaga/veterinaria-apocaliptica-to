#pragma once

#include "cantidad_suministro.h"
#include "dialogos_caso.h"
#include "ficha_dueno.h"
#include "mascota/ficha_mascota.h"

#include <string>
#include <vector>

namespace veterinaria {

struct CasoClinico {
    std::string id;
    FichaMascota mascota;
    FichaDueno dueno;
    DialogosCaso dialogos;
    std::vector<CantidadSuministro> suministros;
    int recompensa;
};

} // namespace veterinaria
