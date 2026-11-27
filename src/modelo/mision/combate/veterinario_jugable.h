#pragma once

#include "../estado_mision.h"

namespace veterinaria {

class VeterinarioJugable {
  public:
    VeterinarioJugable(Punto inicio, int vida_inicial, float velocidad);

    Punto posicion() const;
    int vida() const;
    float velocidad() const;
    EstadoMovimiento movimiento() const;

    /// Guarda la posicion resuelta por combate para este cuadro y actualiza la orientacion.
    void mover_a(Punto posicion);
    void recibir_danio(int cantidad);
    void curar(int cantidad);

  private:
    Punto posicion_;
    int vida_;
    int vida_maxima_;
    float velocidad_;
    EstadoMovimiento movimiento_;
};

} // namespace veterinaria
