#include "veterinario_jugable.h"

#include <algorithm>

namespace veterinaria {

VeterinarioJugable::VeterinarioJugable(Punto inicio, int vida_inicial, float velocidad)
    : posicion_(inicio), vida_(std::max(0, vida_inicial)), velocidad_(std::max(0.0F, velocidad)),
      movimiento_{OrientacionHorizontal::derecha, false} {
}

Punto VeterinarioJugable::posicion() const {
    return posicion_;
}

int VeterinarioJugable::vida() const {
    return vida_;
}

float VeterinarioJugable::velocidad() const {
    return velocidad_;
}

EstadoMovimiento VeterinarioJugable::movimiento() const {
    return movimiento_;
}

void VeterinarioJugable::mover_a(Punto posicion) {
    const float avance_x = posicion.x - posicion_.x;
    const float avance_y = posicion.y - posicion_.y;

    // Sin avance horizontal se conserva la ultima orientacion.
    if (avance_x < 0.0F) {
        movimiento_.orientacion = OrientacionHorizontal::izquierda;
    }

    else if (avance_x > 0.0F) {
        movimiento_.orientacion = OrientacionHorizontal::derecha;
    }

    movimiento_.en_movimiento = avance_x != 0.0F || avance_y != 0.0F;
    posicion_ = posicion;
}

void VeterinarioJugable::recibir_danio(int cantidad) {
    if (cantidad <= 0) {
        return;
    }

    vida_ = std::max(0, vida_ - cantidad);
}

} // namespace veterinaria
