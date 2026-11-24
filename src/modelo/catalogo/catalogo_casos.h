#pragma once

#include "caso_clinico.h"

#include <filesystem>
#include <string>
#include <vector>

namespace veterinaria {

struct ErrorCargaCaso {
    std::filesystem::path archivo;
    std::string detalle;
};

struct CargaCasos {
    std::vector<CasoClinico> casos;
    std::vector<ErrorCargaCaso> errores;
};

class CatalogoCasos {
  public:
    virtual ~CatalogoCasos();

    /// Lee las fichas en orden de nombre de archivo e informa los errores por archivo.
    virtual CargaCasos cargar(const std::filesystem::path& directorio) const = 0;
};

} // namespace veterinaria
