#pragma once

#include "estrategia_enemigo.h"

namespace veterinaria {

class PerseguirPorContacto final : public EstrategiaEnemigo {
  public:
    ~PerseguirPorContacto() override;

    IntencionEnemigo decidir(const EstadoEnemigo& enemigo, Punto jugador, float segundos) override;
};

} // namespace veterinaria
