#pragma once

#include "acciones_controlador.h"

namespace veterinaria {

class Comando {
  public:
    virtual ~Comando();

    virtual void ejecutar(AccionesControlador& receptor) const = 0;
};

} // namespace veterinaria
