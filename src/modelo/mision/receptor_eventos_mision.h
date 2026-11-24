#pragma once

#include "objetivo_mision.h"

namespace veterinaria {

class ReceptorEventosMision {
  public:
    virtual ~ReceptorEventosMision();

    virtual void suministro_recogido(const CantidadSuministro& suministro) = 0;
    virtual void intento_finalizado(DesenlaceMision desenlace) = 0;
};

} // namespace veterinaria
