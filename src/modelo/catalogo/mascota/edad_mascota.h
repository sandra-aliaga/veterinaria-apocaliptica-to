#pragma once

#include <algorithm>

namespace veterinaria::edad_mascota {

constexpr double meses_por_anio = 12.0;
constexpr double edad_humana_primer_anio = 15.0;
constexpr double edad_humana_segundo_anio = 24.0;

/// Convierte meses a anios humanos. El primer anio equivale a 15, el segundo
/// suma 9 y cada anio siguiente suma lo que indique cada especie.
inline double calcular(int edad_meses, double anios_por_anio_adulto) {
    const double anios = std::max(0, edad_meses) / meses_por_anio;

    if (anios <= 1.0) {
        return anios * edad_humana_primer_anio;
    }

    if (anios <= 2.0) {
        return edad_humana_primer_anio +
               (anios - 1.0) * (edad_humana_segundo_anio - edad_humana_primer_anio);
    }

    return edad_humana_segundo_anio + (anios - 2.0) * anios_por_anio_adulto;
}

} // namespace veterinaria::edad_mascota
