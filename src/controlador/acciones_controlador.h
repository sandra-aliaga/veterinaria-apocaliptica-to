#pragma once

namespace veterinaria {

class AccionesControlador {
  public:
    virtual ~AccionesControlador();

    virtual void examinar_mascota() = 0;
    virtual void aceptar_caso() = 0;
    virtual void rechazar_caso() = 0;
    virtual void reintentar_mision() = 0;
    virtual void avanzar_dialogo() = 0;
    virtual void alternar_menu() = 0;
};

} // namespace veterinaria
