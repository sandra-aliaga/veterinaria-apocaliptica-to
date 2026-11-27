#pragma once

#include "../estado_mision.h"
#include "../objetivo_mision.h"

#include <vector>

namespace veterinaria {

class PlanificadorMapa {
  public:
    virtual ~PlanificadorMapa();

    /// Coloca los suministros requeridos, reparte el suelo y reinicia el plan de oleadas.
    virtual Terreno preparar(const PropuestaMision& propuesta) = 0;

    /// Devuelve las apariciones correspondientes al tiempo transcurrido.
    virtual std::vector<Aparicion> avanzar(float segundos) = 0;

    /// Devuelve las apariciones usando la posicion del veterinario y el terreno del cuadro
    /// actual. Si una aparicion no encuentra un lugar valido, se omite.
    virtual std::vector<Aparicion> avanzar(float segundos, Punto jugador, const Terreno& terreno);
};

} // namespace veterinaria
