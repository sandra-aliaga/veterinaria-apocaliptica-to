#include "gato.h"

#include "edad_mascota.h"

#include <utility>

namespace veterinaria {

namespace {

// Desde el tercer anio, cada anio de un gato equivale a cuatro humanos.
constexpr double anios_humanos_por_anio_adulto = 4.0;

} // namespace

Gato::Gato(FichaMascota ficha) : Mascota(std::move(ficha)) {
}

double Gato::calcularEdadHumana() const {
    return edad_mascota::calcular(ficha().edad_meses, anios_humanos_por_anio_adulto);
}

} // namespace veterinaria
