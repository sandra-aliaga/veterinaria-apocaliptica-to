#pragma once

namespace veterinaria {

class Arma {
  public:
    virtual ~Arma();

    virtual void reiniciar() = 0;
    virtual void actualizar(float segundos) = 0;
    virtual float alcance() const = 0;

    /// Combate consulta en cada paso. Devuelve cero mientras el arma recarga.
    virtual int intentar_atacar() = 0;
};

} // namespace veterinaria
