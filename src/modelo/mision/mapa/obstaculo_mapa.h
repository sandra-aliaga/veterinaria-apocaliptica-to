#pragma once

#include "../geometria_mapa.h"

namespace veterinaria {

enum class TipoObstaculo { arbol, roca, tronco };

struct ObstaculoMapa {
    TipoObstaculo tipo;
    Punto anclaje;
    Rectangulo colision;
};

} // namespace veterinaria
