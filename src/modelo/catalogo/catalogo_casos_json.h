#pragma once

#include "catalogo_casos.h"
#include "mascota/fabrica_mascotas.h"

namespace veterinaria {

/// Carga una ficha por archivo .json del directorio, en orden de nombre.
///
/// La fabrica decide que especies se admiten, asi que agregar una especie no
/// obliga a cambiar el catalogo. Debe vivir mientras se use el catalogo.
class CatalogoCasosJson final : public CatalogoCasos {
  public:
    explicit CatalogoCasosJson(const FabricaMascotas& fabrica);
    ~CatalogoCasosJson() override;

    CargaCasos cargar(const std::filesystem::path& directorio) const override;

    /// Interpreta una ficha. Lanza std::invalid_argument con el campo que falla.
    CasoClinico interpretar(const std::string& texto) const;

  private:
    const FabricaMascotas& fabrica_;
};

} // namespace veterinaria
