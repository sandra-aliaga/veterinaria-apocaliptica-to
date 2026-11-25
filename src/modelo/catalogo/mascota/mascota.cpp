#include "mascota.h"

#include <utility>

namespace veterinaria {

Mascota::Mascota(FichaMascota ficha) : ficha_(std::move(ficha)) {
}

Mascota::~Mascota() = default;

const FichaMascota& Mascota::ficha() const {
    return ficha_;
}

} // namespace veterinaria
