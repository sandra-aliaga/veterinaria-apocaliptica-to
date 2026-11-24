#pragma once

#include "../estado_mision.h"
#include "../objetivo_mision.h"

#include <vector>

namespace veterinaria {

class PlanificadorMapa {
  public:
    virtual ~PlanificadorMapa();

    /// Coloca los suministros requeridos y reinicia el plan de oleadas.
    virtual Terreno preparar(const PropuestaMision& propuesta) = 0;

    /// Devuelve las apariciones correspondientes al tiempo transcurrido.
    virtual std::vector<Aparicion> avanzar(float segundos) = 0;
};

} // namespace veterinaria
