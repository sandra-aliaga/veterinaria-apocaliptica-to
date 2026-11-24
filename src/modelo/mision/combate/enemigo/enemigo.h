#pragma once

#include "estrategia_enemigo.h"

namespace veterinaria {

class Enemigo {
  public:
    virtual ~Enemigo();

    virtual EstadoEnemigo estado() const = 0;
    virtual IntencionEnemigo decidir(Punto jugador, float segundos) = 0;
    virtual void mover_a(Punto posicion) = 0;
    virtual void recibir_danio(int cantidad) = 0;
};

} // namespace veterinaria
