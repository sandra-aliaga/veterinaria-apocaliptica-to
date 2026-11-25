#pragma once

#include "combate/combate.h"
#include "mapa/planificador_mapa.h"
#include "mision.h"

#include <optional>
#include <string>
#include <vector>

namespace veterinaria {

/// Coordina un intento: prepara el terreno, avanza combate y oleadas, mide el reloj,
/// resuelve la recoleccion y entrega el resultado una sola vez.
class MisionBase final : public Mision {
  public:
    /// El planificador y el combate deben vivir mas que la mision.
    /// `radio_recoleccion` es la distancia al centro del suministro que cuenta como contacto.
    MisionBase(PlanificadorMapa& planificador, Combate& combate, float radio_recoleccion = 16.0F);

    void iniciar(const PropuestaMision& propuesta) override;
    void actualizar(Punto direccion, float segundos) override;
    const EstadoMision& estado() const override;
    std::optional<ResultadoMision> extraer_resultado() override;

    void suscribir(ReceptorEventosMision& receptor) override;
    void desuscribir(ReceptorEventosMision& receptor) override;

  private:
    void recolectar();
    bool objetivo_cubierto() const;
    void finalizar(DesenlaceMision desenlace, std::optional<MotivoFalloMision> motivo);

    PlanificadorMapa& planificador_;
    Combate& combate_;
    float radio_recoleccion_;

    EstadoMision estado_;
    std::string id_caso_;
    bool activa_;
    std::optional<ResultadoMision> resultado_;
    std::vector<ReceptorEventosMision*> receptores_;
};

} // namespace veterinaria