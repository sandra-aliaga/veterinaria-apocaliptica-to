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
    std::vector<Aparicion> avanzar(float segundos, Punto jugador, const Terreno& terreno) override;

  private:
    /// El plan fija cuando sale cada enemigo y por que lado de la camara prefiere entrar
    /// (0 arriba, 1 derecha, 2 abajo, 3 izquierda). La posicion se elige al vencer.
    struct AparicionProgramada {
        float segundo;
        TipoEnemigo tipo;
        int lado;
    };

    Rectangulo limites_;
    std::optional<std::uint32_t> semilla_fija_;
    std::mt19937 generador_;
    float segundos_transcurridos_;
    Terreno terreno_;
    std::vector<AparicionProgramada> oleadas_;
    std::size_t siguiente_aparicion_;
};

} // namespace veterinaria
