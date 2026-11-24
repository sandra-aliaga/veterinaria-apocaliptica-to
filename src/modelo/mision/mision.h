#pragma once

#include "estado_mision.h"
#include "objetivo_mision.h"
#include "receptor_eventos_mision.h"

#include <optional>

namespace veterinaria {

class Mision {
  public:
    virtual ~Mision();

    virtual void iniciar(const PropuestaMision& propuesta) = 0;
    virtual void actualizar(Punto direccion, float segundos) = 0;
    virtual const EstadoMision& estado() const = 0;
    virtual std::optional<ResultadoMision> extraer_resultado() = 0;

    /// El receptor debe darse de baja antes de destruirse.
    virtual void suscribir(ReceptorEventosMision& receptor) = 0;
    virtual void desuscribir(ReceptorEventosMision& receptor) = 0;
};

} // namespace veterinaria
