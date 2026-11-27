#pragma once

#include "estrategia_enemigo.h"

namespace veterinaria {

class AtacarADistancia final : public EstrategiaEnemigo {
  public:
    AtacarADistancia(float alcance, float intervalo_entre_disparos, bool mantener_puesto = false);
    ~AtacarADistancia() override;

    IntencionEnemigo decidir(const EstadoEnemigo& enemigo, Punto jugador, float segundos) override;

  private:
    float alcance_;
    float intervalo_entre_disparos_;
    float segundos_para_disparar_;
    bool mantener_puesto_;
};

} // namespace veterinaria
