#pragma once

#include "../estado_mision.h"

namespace veterinaria {

class Combate {
  public:
    virtual ~Combate();

    virtual void iniciar(const Terreno& terreno) = 0;
    virtual void agregar_enemigo(const Aparicion& aparicion) = 0;
    virtual void actualizar(Punto direccion, float segundos) = 0;

    /// La referencia sigue valida hasta la siguiente modificacion del combate.
    virtual const EstadoCombate& estado() const = 0;
};

} // namespace veterinaria
