#include "perseguir_por_contacto.h"

#include "../geometria_combate.h"

namespace veterinaria {

PerseguirPorContacto::~PerseguirPorContacto() = default;

IntencionEnemigo PerseguirPorContacto::decidir(const EstadoEnemigo& enemigo, Punto jugador,
                                               float /*segundos*/) {
    // Corre directo hacia el veterinario. Combate aplica el dano al tocarlo.
    return {geometria_combate::direccion_hacia(enemigo.posicion, jugador), AtaqueEnemigo::contacto};
}

} // namespace veterinaria
