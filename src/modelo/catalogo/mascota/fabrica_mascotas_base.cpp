#include "fabrica_mascotas_base.h"

#include "gato.h"
#include "perro.h"

namespace veterinaria {

FabricaMascotasBase::~FabricaMascotasBase() = default;

std::unique_ptr<Mascota> FabricaMascotasBase::crear(const FichaMascota& ficha) const {
    if (ficha.especie == "gato") {
        return std::make_unique<Gato>(ficha);
    }

    if (ficha.especie == "perro") {
        return std::make_unique<Perro>(ficha);
    }

    return nullptr;
}

} // namespace veterinaria
