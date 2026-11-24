#pragma once

#include "../../estado_mision.h"

namespace veterinaria {

enum class AtaqueEnemigo { ninguno, contacto, proyectil };

struct IntencionEnemigo {
    Punto direccion;
    AtaqueEnemigo ataque;
};

class EstrategiaEnemigo {
  public:
    virtual ~EstrategiaEnemigo();

    virtual IntencionEnemigo decidir(const EstadoEnemigo& enemigo, Punto jugador,
                                     float segundos) = 0;
};

} // namespace veterinaria
