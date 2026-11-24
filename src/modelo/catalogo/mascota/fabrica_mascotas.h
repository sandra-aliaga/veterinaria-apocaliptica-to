#pragma once

#include "mascota.h"

#include <memory>

namespace veterinaria {

class FabricaMascotas {
  public:
    virtual ~FabricaMascotas();

    /// Devuelve nulo si la especie aun no esta admitida.
    virtual std::unique_ptr<Mascota> crear(const FichaMascota& ficha) const = 0;
};

} // namespace veterinaria
