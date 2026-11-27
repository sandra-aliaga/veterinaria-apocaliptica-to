#include "atacar_a_distancia.h"

#include "../geometria_combate.h"

#include <algorithm>

namespace veterinaria {

namespace {

// Por debajo de esta fraccion del alcance el superviviente retrocede.
constexpr float fraccion_distancia_minima = 0.45F;

} // namespace

AtacarADistancia::AtacarADistancia(float alcance, float intervalo_entre_disparos,
                                   bool mantener_puesto)
    : alcance_(std::max(0.0F, alcance)),
      intervalo_entre_disparos_(std::max(0.0F, intervalo_entre_disparos)),
      segundos_para_disparar_(intervalo_entre_disparos_), mantener_puesto_(mantener_puesto) {
}

AtacarADistancia::~AtacarADistancia() = default;

IntencionEnemigo AtacarADistancia::decidir(const EstadoEnemigo& enemigo, Punto jugador,
                                           float segundos) {
    segundos_para_disparar_ = std::max(0.0F, segundos_para_disparar_ - std::max(0.0F, segundos));

    const float distancia = geometria_combate::distancia(enemigo.posicion, jugador);
    const Punto hacia_jugador = geometria_combate::direccion_hacia(enemigo.posicion, jugador);

    IntencionEnemigo intencion{{0.0F, 0.0F}, AtaqueEnemigo::ninguno};

    // Mantiene distancia: se acerca si esta lejos y retrocede si esta demasiado cerca.
    if (!mantener_puesto_ && distancia > alcance_) {
        intencion.direccion = hacia_jugador;
    }

    else if (!mantener_puesto_ && distancia < alcance_ * fraccion_distancia_minima) {
        intencion.direccion = {-hacia_jugador.x, -hacia_jugador.y};
    }

    if (distancia <= alcance_ && segundos_para_disparar_ <= 0.0F) {
        intencion.ataque = AtaqueEnemigo::proyectil;
        segundos_para_disparar_ = intervalo_entre_disparos_;
    }

    return intencion;
}

} // namespace veterinaria
