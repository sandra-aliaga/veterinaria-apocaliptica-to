#pragma once

#include <string>

namespace veterinaria {

struct FichaMascota {
    std::string id;
    std::string nombre;
    std::string especie;
    int edad_meses;
    std::string retrato;
};

} // namespace veterinaria
