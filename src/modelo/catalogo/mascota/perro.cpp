#include "perro.h"

#include "edad_mascota.h"

#include <utility>

namespace veterinaria {

namespace {

// Desde el tercer anio, cada anio de un perro equivale a cinco humanos.
constexpr double anios_humanos_por_anio_adulto = 5.0;

} // namespace

Perro::Perro(FichaMascota ficha) : Mascota(std::move(ficha)) {
}

double Perro::calcularEdadHumana() const {
    return edad_mascota::calcular(ficha().edad_meses, anios_humanos_por_anio_adulto);
}

} // namespace veterinaria
