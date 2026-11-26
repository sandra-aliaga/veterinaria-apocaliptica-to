#include "fabrica_enemigos_base.h"

#include "atacar_a_distancia.h"
#include "enemigo_con_estrategia.h"
#include "perseguir_por_contacto.h"

namespace veterinaria {

namespace {

// Unidades: pixeles del mundo por segundo y puntos de vida.
constexpr float alcance_disparo = 220.0F;
constexpr float intervalo_entre_disparos = 2.0F;

std::unique_ptr<EstrategiaEnemigo> crear_estrategia(TipoEnemigo tipo) {
    switch (tipo) {
    case TipoEnemigo::superviviente_a_distancia:
        return std::make_unique<AtacarADistancia>(alcance_disparo, intervalo_entre_disparos);
    case TipoEnemigo::animal_de_contacto:
    case TipoEnemigo::superviviente_de_contacto:
        break;
    }

    return std::make_unique<PerseguirPorContacto>();
}

} // namespace

FabricaEnemigosBase::~FabricaEnemigosBase() = default;

PerfilEnemigo FabricaEnemigosBase::perfil(TipoEnemigo tipo) {
    switch (tipo) {
    case TipoEnemigo::animal_de_contacto:
        return {30, 90.0F, 10}; // Rapido y fragil.
    case TipoEnemigo::superviviente_de_contacto:
        return {60, 60.0F, 15}; // Lento y resistente.
    case TipoEnemigo::superviviente_a_distancia:
        return {40, 50.0F, 8}; // El dano lo aplica cada proyectil.
    }

    return {30, 90.0F, 10};
}

std::unique_ptr<Enemigo> FabricaEnemigosBase::crear(const Aparicion& aparicion) const {
    const PerfilEnemigo valores = perfil(aparicion.tipo);
    const EstadoEnemigo estado{aparicion.tipo, aparicion.posicion,
                               valores.vida,   valores.velocidad,
                               valores.danio,  {OrientacionHorizontal::derecha, false}};

    return std::make_unique<EnemigoConEstrategia>(estado, crear_estrategia(aparicion.tipo));
}

} // namespace veterinaria
