#pragma once

#include "../geometria_mapa.h"

#include <cmath>

namespace veterinaria::geometria_combate {

inline float distancia(Punto a, Punto b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

/// Devuelve la direccion unitaria de origen a destino, o cero si coinciden.
inline Punto direccion_hacia(Punto origen, Punto destino) {
    const float dx = destino.x - origen.x;
    const float dy = destino.y - origen.y;
    const float longitud = std::hypot(dx, dy);

    if (longitud <= 0.0F) {
        return {0.0F, 0.0F};
    }

    return {dx / longitud, dy / longitud};
}

/// Normaliza la direccion para que las diagonales no sean mas rapidas.
inline Punto normalizar(Punto direccion) {
    return direccion_hacia({0.0F, 0.0F}, direccion);
}

} // namespace veterinaria::geometria_combate
