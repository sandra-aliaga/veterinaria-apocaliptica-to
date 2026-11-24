#pragma once

#include "enemigo.h"

#include <memory>

namespace veterinaria {

class EnemigoConEstrategia final : public Enemigo {
  public:
    EnemigoConEstrategia(EstadoEnemigo estado, std::unique_ptr<EstrategiaEnemigo> estrategia);
    ~EnemigoConEstrategia() override;

    EstadoEnemigo estado() const override;
    IntencionEnemigo decidir(Punto jugador, float segundos) override;
    void mover_a(Punto posicion) override;
    void recibir_danio(int cantidad) override;

  private:
    EstadoEnemigo estado_;
    std::unique_ptr<EstrategiaEnemigo> estrategia_;
};

} // namespace veterinaria
