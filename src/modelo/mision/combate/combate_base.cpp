#include "combate_base.h"

#include "geometria_combate.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace veterinaria {

namespace {

using geometria_combate::direccion_hacia;
using geometria_combate::distancia;
using geometria_combate::normalizar;

/// Indica si un circulo toca un rectangulo.
bool toca_rectangulo(Punto centro, float radio, const Rectangulo& rect) {
    const float cercano_x = std::clamp(centro.x, rect.origen.x, rect.origen.x + rect.ancho);
    const float cercano_y = std::clamp(centro.y, rect.origen.y, rect.origen.y + rect.alto);
    return distancia(centro, {cercano_x, cercano_y}) < radio;
}

/// Indica si un circulo cabe completo dentro de los limites.
bool dentro_de_limites(Punto centro, float radio, const Rectangulo& limites) {
    return centro.x - radio >= limites.origen.x &&
           centro.x + radio <= limites.origen.x + limites.ancho &&
           centro.y - radio >= limites.origen.y &&
           centro.y + radio <= limites.origen.y + limites.alto;
}

OrientacionHorizontal orientacion_hacia(float diferencia, OrientacionHorizontal actual) {
    if (diferencia == 0.0F) {
        return actual;
    }

    return diferencia < 0.0F ? OrientacionHorizontal::izquierda : OrientacionHorizontal::derecha;
}

} // namespace

CombateBase::CombateBase(std::unique_ptr<FabricaEnemigos> fabrica, std::unique_ptr<Arma> arma,
                         ConfiguracionCombate configuracion)
    : fabrica_(std::move(fabrica)), arma_(std::move(arma)), configuracion_(configuracion),
      terreno_{},
      veterinario_({0.0F, 0.0F}, configuracion.vida_jugador, configuracion.velocidad_jugador),
      estado_{} {
    publicar_estado();
}

CombateBase::~CombateBase() = default;

void CombateBase::iniciar(const Terreno& terreno) {
    terreno_ = terreno;
    veterinario_ = VeterinarioJugable(terreno.inicio, configuracion_.vida_jugador,
                                      configuracion_.velocidad_jugador);
    enemigos_.clear();
    proyectiles_.clear();
    alternar_animal_ = false;
    alternar_superviviente_ = false;
    estado_ = {};

    if (arma_) {
        arma_->reiniciar();
    }

    publicar_estado();
}

void CombateBase::agregar_enemigo(const Aparicion& aparicion) {
    if (!fabrica_) {
        return;
    }

    auto enemigo = fabrica_->crear(aparicion);

    if (enemigo) {
        bool variante = false;

        if (aparicion.tipo == TipoEnemigo::animal_de_contacto) {
            variante = alternar_animal_;
            alternar_animal_ = !alternar_animal_;
        }

        else if (aparicion.tipo == TipoEnemigo::superviviente_de_contacto) {
            variante = alternar_superviviente_;
            alternar_superviviente_ = !alternar_superviviente_;
        }

        enemigos_.push_back({std::move(enemigo), 0.0F, variante, std::nullopt});
        publicar_estado();
    }
}

void CombateBase::actualizar(Punto direccion, float segundos) {
    const Punto unitaria = normalizar(direccion);
    float restante = std::max(0.0F, segundos);

    // Pasos cortos para que las entidades no atraviesen obstaculos ni al jugador.
    while (restante > 0.0F && veterinario_.vida() > 0) {
        const float paso = std::min(restante, configuracion_.paso_maximo_segundos);
        avanzar(unitaria, paso);
        restante -= paso;
    }

    if (segundos <= 0.0F || veterinario_.vida() <= 0) {
        // Sin avance el jugador queda quieto, pero conserva su orientacion.
        veterinario_.mover_a(veterinario_.posicion());
    }

    publicar_estado();
}

const EstadoCombate& CombateBase::estado() const {
    return estado_;
}

void CombateBase::avanzar(Punto direccion, float segundos) {
    estado_.segundos_combate += segundos;
    const int vida_anterior = veterinario_.vida();

    if (arma_) {
        arma_->actualizar(segundos);
    }

    mover_jugador(direccion, segundos);
    actualizar_enemigos(segundos);
    actualizar_proyectiles(segundos);

    if (veterinario_.vida() < vida_anterior) {
        estado_.instante_danio = estado_.segundos_combate;
    }

    atacar_con_arma();
    retirar_eliminados();
}

void CombateBase::mover_jugador(Punto direccion, float segundos) {
    const float paso = veterinario_.velocidad() * segundos;
    const Punto avance{direccion.x * paso, direccion.y * paso};
    veterinario_.mover_a(
        resolver_movimiento(veterinario_.posicion(), avance, configuracion_.radio_jugador));
}

void CombateBase::actualizar_enemigos(float segundos) {
    const Punto jugador = veterinario_.posicion();

    for (auto& activo : enemigos_) {
        activo.segundos_para_danio = std::max(0.0F, activo.segundos_para_danio - segundos);

        const IntencionEnemigo intencion = activo.enemigo->decidir(jugador, segundos);
        const EstadoEnemigo actual = activo.enemigo->estado();
        const Punto unitaria = normalizar(intencion.direccion);
        const float paso = actual.velocidad * segundos;
        Punto destino = resolver_movimiento(actual.posicion, {unitaria.x * paso, unitaria.y * paso},
                                            configuracion_.radio_enemigo);

        // El enemigo se queda pegado al borde del veterinario en lugar de encimarse.
        const float separacion = configuracion_.radio_jugador + configuracion_.radio_enemigo;
        if (distancia(destino, jugador) < separacion &&
            distancia(destino, jugador) < distancia(actual.posicion, jugador)) {
            const Punto hacia = direccion_hacia(actual.posicion, jugador);
            const Punto borde{jugador.x - hacia.x * separacion, jugador.y - hacia.y * separacion};
            destino = es_transitable(borde, configuracion_.radio_enemigo) ? borde : actual.posicion;
        }

        activo.enemigo->mover_a(destino);

        if (intencion.ataque == AtaqueEnemigo::contacto && en_contacto(destino, 0.0F) &&
            activo.segundos_para_danio <= 0.0F) {
            veterinario_.recibir_danio(actual.danio);
            activo.segundos_para_danio = configuracion_.intervalo_danio_contacto;
        }

        else if (intencion.ataque == AtaqueEnemigo::proyectil) {
            const Punto hacia = direccion_hacia(destino, jugador);
            const auto orientacion = orientacion_hacia(hacia.x, actual.movimiento.orientacion);
            activo.disparo = GestoCombate{orientacion, estado_.segundos_combate};
            proyectiles_.push_back({destino,
                                    {hacia.x * configuracion_.velocidad_proyectil,
                                     hacia.y * configuracion_.velocidad_proyectil},
                                    actual.danio,
                                    configuracion_.duracion_proyectil});
        }
    }
}

void CombateBase::actualizar_proyectiles(float segundos) {
    const float alcance_impacto = configuracion_.radio_jugador + configuracion_.radio_proyectil;

    const auto termina = [&](Proyectil& proyectil) {
        proyectil.segundos_restantes -= segundos;
        proyectil.posicion.x += proyectil.velocidad.x * segundos;
        proyectil.posicion.y += proyectil.velocidad.y * segundos;

        if (distancia(proyectil.posicion, veterinario_.posicion()) <= alcance_impacto) {
            veterinario_.recibir_danio(proyectil.danio);
            return true;
        }

        return proyectil.segundos_restantes <= 0.0F ||
               !es_transitable(proyectil.posicion, configuracion_.radio_proyectil);
    };

    proyectiles_.erase(std::remove_if(proyectiles_.begin(), proyectiles_.end(), termina),
                       proyectiles_.end());
}

void CombateBase::atacar_con_arma() {
    if (!arma_ || veterinario_.vida() <= 0) {
        return;
    }

    // Golpea solo al enemigo mas cercano entre los que estan al alcance.
    EnemigoActivo* objetivo = nullptr;
    float mas_cercano = std::numeric_limits<float>::max();

    for (auto& activo : enemigos_) {
        const EstadoEnemigo actual = activo.enemigo->estado();
        const float separacion = distancia(actual.posicion, veterinario_.posicion());

        if (actual.vida > 0 && en_contacto(actual.posicion, arma_->alcance()) &&
            separacion < mas_cercano) {
            mas_cercano = separacion;
            objetivo = &activo;
        }
    }

    if (objetivo == nullptr) {
        return;
    }

    const int danio = arma_->intentar_atacar();

    if (danio > 0) {
        const auto impacto = objetivo->enemigo->estado().posicion;
        const auto orientacion = orientacion_hacia(impacto.x - veterinario_.posicion().x,
                                                   veterinario_.movimiento().orientacion);
        estado_.ultimo_golpe = GolpeArma{impacto, {orientacion, estado_.segundos_combate}};
        objetivo->enemigo->recibir_danio(danio);
    }
}

void CombateBase::retirar_eliminados() {
    enemigos_.erase(std::remove_if(enemigos_.begin(), enemigos_.end(),
                                   [](const EnemigoActivo& activo) {
                                       return activo.enemigo->estado().vida <= 0;
                                   }),
                    enemigos_.end());
}

void CombateBase::publicar_estado() {
    estado_.jugador = veterinario_.posicion();
    estado_.vida = veterinario_.vida();
    estado_.vida_maxima = configuracion_.vida_jugador;
    estado_.movimiento_jugador = veterinario_.movimiento();

    estado_.enemigos.clear();
    for (const auto& activo : enemigos_) {
        auto enemigo = activo.enemigo->estado();
        enemigo.variante_visual = activo.variante_visual;
        enemigo.disparo = activo.disparo;
        estado_.enemigos.push_back(enemigo);
    }

    estado_.proyectiles.clear();
    for (const auto& proyectil : proyectiles_) {
        estado_.proyectiles.push_back({proyectil.posicion, proyectil.velocidad});
    }
}

Punto CombateBase::resolver_movimiento(Punto desde, Punto avance, float radio) const {
    // Se resuelve cada eje por separado para deslizarse junto a los obstaculos.
    Punto resultado = desde;

    const Punto en_x{resultado.x + avance.x, resultado.y};
    if (es_transitable(en_x, radio)) {
        resultado = en_x;
    }

    const Punto en_y{resultado.x, resultado.y + avance.y};
    if (es_transitable(en_y, radio)) {
        resultado = en_y;
    }

    return resultado;
}

bool CombateBase::es_transitable(Punto centro, float radio) const {
    if (!dentro_de_limites(centro, radio, terreno_.limites)) {
        return false;
    }

    // Solo bloquea la zona solida de cada obstaculo, no su sprite completo.
    return std::none_of(terreno_.obstaculos.begin(), terreno_.obstaculos.end(),
                        [&](const ObstaculoMapa& obstaculo) {
                            return toca_rectangulo(centro, radio, obstaculo.colision);
                        });
}

bool CombateBase::en_contacto(Punto enemigo, float alcance_extra) const {
    // El alcance se mide desde los limites de contacto de ambos cuerpos.
    const float limite =
        configuracion_.radio_jugador + configuracion_.radio_enemigo + alcance_extra + 0.5F;
    return distancia(enemigo, veterinario_.posicion()) <= limite;
}

} // namespace veterinaria
