#pragma once

#include "../catalogo/caso_clinico.h"
#include "../mision/objetivo_mision.h"

#include <optional>

namespace veterinaria {

enum class RegistroMision { sin_caso_activo, resultado_invalido, fallo_registrado, tratado };

class Consultorio {
  public:
    virtual ~Consultorio();

    virtual void recibir(CasoClinico caso) = 0;

    /// Los punteros consultados se usan antes de volver a modificar el consultorio.
    virtual const CasoClinico* ficha_pendiente() const = 0;
    virtual const CasoClinico* caso_activo() const = 0;

    virtual std::optional<PropuestaMision> aceptar() = 0;
    virtual bool rechazar() = 0;
    virtual std::optional<PropuestaMision> reintentar() = 0;

    /// Valida todos los suministros y realiza un solo cobro por caso tratado.
    virtual RegistroMision registrar_resultado(const ResultadoMision& resultado) = 0;

    virtual int saldo() const = 0;
};

} // namespace veterinaria
