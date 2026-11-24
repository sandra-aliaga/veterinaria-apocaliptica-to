#pragma once

#include "acciones_controlador.h"
#include "estado_pantalla.h"

namespace veterinaria {

class Controlador : public AccionesControlador {
  public:
    ~Controlador() override;

    virtual void actualizar(const EntradaCuadro& entrada, float segundos) = 0;
    virtual EstadoPantalla estado_pantalla() const = 0;
};

} // namespace veterinaria
