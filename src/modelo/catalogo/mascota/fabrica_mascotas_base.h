#pragma once

#include "fabrica_mascotas.h"

namespace veterinaria {

/// Crea un Gato o un Perro segun la especie de la ficha ("gato" o "perro").
class FabricaMascotasBase final : public FabricaMascotas {
  public:
    ~FabricaMascotasBase() override;

    std::unique_ptr<Mascota> crear(const FichaMascota& ficha) const override;
};

} // namespace veterinaria
