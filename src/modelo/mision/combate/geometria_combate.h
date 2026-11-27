#pragma once

#include "../geometria_mapa.h"

#include <algorithm>
#include <cmath>

namespace veterinaria::geometria_combate {

inline float distancia(Punto a, Punto b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

inline float distancia_a_segmento(Punto punto, Punto inicio, Punto fin) {
    const float dx = fin.x - inicio.x;
    const float dy = fin.y - inicio.y;
    const float longitud_cuadrada = dx * dx + dy * dy;

    if (longitud_cuadrada == 0.0F) {
        return distancia(punto, inicio);
    }

    const float producto = (punto.x - inicio.x) * dx + (punto.y - inicio.y) * dy;
    const float avance = std::clamp(producto / longitud_cuadrada, 0.0F, 1.0F);
    return distancia(punto, {inicio.x + avance * dx, inicio.y + avance * dy});
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
