#pragma once

#include "../controlador/estado_pantalla.h"

namespace veterinaria {

class Vista {
  public:
    virtual ~Vista();

    virtual EntradaCuadro leer_entrada() = 0;
    virtual void dibujar(const EstadoPantalla& estado) = 0;
};

} // namespace veterinaria
