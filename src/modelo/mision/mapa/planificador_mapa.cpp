#include "planificador_mapa.h"

namespace veterinaria {

PlanificadorMapa::~PlanificadorMapa() = default;

std::vector<Aparicion> PlanificadorMapa::avanzar(float segundos, Punto, const Terreno&) {
    return avanzar(segundos);
}

} // namespace veterinaria