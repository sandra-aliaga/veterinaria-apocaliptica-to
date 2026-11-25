#pragma once

#include "planificador_mapa.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace veterinaria {

class PlanificadorCampoAbierto final : public PlanificadorMapa {
  public:
    explicit PlanificadorCampoAbierto(Rectangulo limites,
                                      std::optional<std::uint32_t> semilla_fija = std::nullopt);

    Terreno preparar(const PropuestaMision& propuesta) override;
    std::vector<Aparicion> avanzar(float segundos) override;

  private:
    struct AparicionProgramada {
        float segundo;
        Aparicion aparicion;
    };

    Rectangulo limites_;
    std::optional<std::uint32_t> semilla_fija_;
    std::mt19937 generador_;
    float segundos_transcurridos_;
    std::vector<AparicionProgramada> oleadas_;
    std::size_t siguiente_aparicion_;
};

} // namespace veterinaria
