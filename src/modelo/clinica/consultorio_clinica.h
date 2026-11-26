#pragma once

#include "cola_casos.h"
#include "consultorio.h"

#include <optional>

namespace veterinaria {

/// Consultorio concreto: guarda las fichas en una cola FIFO, mantiene el caso activo y el saldo.
class ConsultorioClinica : public Consultorio {
  public:
    ConsultorioClinica();
    ~ConsultorioClinica() override;

    void recibir(CasoClinico caso) override;

    const CasoClinico* ficha_pendiente() const override;
    const CasoClinico* caso_activo() const override;

    std::optional<PropuestaMision> aceptar() override;
    bool rechazar() override;
    std::optional<PropuestaMision> reintentar() override;

    RegistroMision registrar_resultado(const ResultadoMision& resultado) override;

    int saldo() const override;

  private:
    PropuestaMision proponer_mision() const;

    ColaCasosClinicos cola_;
    std::optional<CasoClinico> activo_;
    /// Verdadero mientras el intento actual espera su resultado; solo existe uno a la vez.
    bool intento_en_curso_;
    int saldo_;
};

} // namespace veterinaria
