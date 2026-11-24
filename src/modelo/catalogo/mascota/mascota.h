#pragma once

#include "ficha_mascota.h"

namespace veterinaria {

class Mascota {
  public:
    virtual ~Mascota();

    const FichaMascota& ficha() const;
    virtual double calcularEdadHumana() const = 0;

  protected:
    explicit Mascota(FichaMascota ficha);

  private:
    FichaMascota ficha_;
};

} // namespace veterinaria
