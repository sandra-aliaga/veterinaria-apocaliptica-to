#pragma once

#include "mascota.h"

namespace veterinaria {

class Perro : public Mascota {
  public:
    explicit Perro(FichaMascota ficha);

    double calcularEdadHumana() const override;
};

} // namespace veterinaria
