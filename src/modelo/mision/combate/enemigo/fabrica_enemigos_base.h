#pragma once

#include "fabrica_enemigos.h"

namespace veterinaria {

/// Valores de un tipo de enemigo. La fabrica los copia en el estado inicial.
struct PerfilEnemigo {
    int vida;
    float velocidad;
    int danio;
};

/// Crea cada enemigo con su perfil y elige la estrategia segun su tipo.
class FabricaEnemigosBase final : public FabricaEnemigos {
  public:
    ~FabricaEnemigosBase() override;

    std::unique_ptr<Enemigo> crear(const Aparicion& aparicion) const override;

    static PerfilEnemigo perfil(TipoEnemigo tipo);
};

} // namespace veterinaria
