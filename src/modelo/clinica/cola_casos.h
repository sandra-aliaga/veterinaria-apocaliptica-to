#pragma once

#include "../catalogo/caso_clinico.h"

#include <memory>
#include <optional>

namespace veterinaria {

class ColaCasosClinicos {
  public:
    ColaCasosClinicos();
    ~ColaCasosClinicos();
    ColaCasosClinicos(const ColaCasosClinicos&) = delete;
    ColaCasosClinicos& operator=(const ColaCasosClinicos&) = delete;

    void encolar(CasoClinico caso);
    std::optional<CasoClinico> extraer();

    /// La ficha prestada es valida hasta retirarla o destruir la cola.
    const CasoClinico* frente() const;

  private:
    struct Nodo;

    std::unique_ptr<Nodo> primero_;
    Nodo* ultimo_;
};

} // namespace veterinaria
