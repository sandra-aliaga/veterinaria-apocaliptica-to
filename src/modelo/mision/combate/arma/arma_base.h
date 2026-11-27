#pragma once

#include "arma.h"

namespace veterinaria {

class ArmaBase : public Arma {
  public:
    ArmaBase(int danio, float intervalo_entre_ataques, float alcance);

    void reiniciar() override;
    void actualizar(float segundos) override;
    float alcance() const override;
    int intentar_atacar() override;

  private:
    int danio_;
    float alcance_;
    float intervalo_entre_ataques_;
    float segundos_para_atacar_;
};

} // namespace veterinaria
