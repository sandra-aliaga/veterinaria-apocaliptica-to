#pragma once

namespace veterinaria {

class Arma {
  public:
    virtual ~Arma();

    virtual void reiniciar() = 0;
    virtual void actualizar(float segundos) = 0;
    virtual float alcance() const = 0;

    /// Combate llama a este metodo con un objetivo al alcance. Cero indica recarga.
    virtual int intentar_atacar() = 0;
};

} // namespace veterinaria
