#include "enemigo_con_estrategia.h"

#include <algorithm>
#include <utility>

namespace veterinaria {

EnemigoConEstrategia::EnemigoConEstrategia(EstadoEnemigo estado,
                                           std::unique_ptr<EstrategiaEnemigo> estrategia)
    : estado_(estado), estrategia_(std::move(estrategia)) {
}

EnemigoConEstrategia::~EnemigoConEstrategia() = default;

EstadoEnemigo EnemigoConEstrategia::estado() const {
    return estado_;
}

IntencionEnemigo EnemigoConEstrategia::decidir(Punto jugador, float segundos) {
    if (!estrategia_ || estado_.vida <= 0) {
        return {{0.0F, 0.0F}, AtaqueEnemigo::ninguno};
    }

    return estrategia_->decidir(estado_, jugador, segundos);
}

void EnemigoConEstrategia::mover_a(Punto posicion) {
    const float avance_x = posicion.x - estado_.posicion.x;
    const float avance_y = posicion.y - estado_.posicion.y;

    if (avance_x < 0.0F) {
        estado_.movimiento.orientacion = OrientacionHorizontal::izquierda;
    }

    else if (avance_x > 0.0F) {
        estado_.movimiento.orientacion = OrientacionHorizontal::derecha;
    }

    estado_.movimiento.en_movimiento = avance_x != 0.0F || avance_y != 0.0F;
    estado_.posicion = posicion;
}

void EnemigoConEstrategia::recibir_danio(int cantidad) {
    if (cantidad <= 0) {
        return;
    }

    estado_.vida = std::max(0, estado_.vida - cantidad);
}

} // namespace veterinaria
