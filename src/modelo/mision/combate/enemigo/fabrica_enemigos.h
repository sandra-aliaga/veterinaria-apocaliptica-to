#pragma once

#include "enemigo.h"

#include <memory>

namespace veterinaria {

class FabricaEnemigos {
  public:
    virtual ~FabricaEnemigos();

    virtual std::unique_ptr<Enemigo> crear(const Aparicion& aparicion) const = 0;
};

} // namespace veterinaria
