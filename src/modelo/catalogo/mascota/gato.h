#pragma once

#include "mascota.h"

namespace veterinaria {

class Gato : public Mascota {
  public:
    explicit Gato(FichaMascota ficha);

    double calcularEdadHumana() const override;
};

} // namespace veterinaria
