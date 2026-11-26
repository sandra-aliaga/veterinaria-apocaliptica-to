#include "mision_base.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace veterinaria {

namespace {

int cantidad_de(const std::vector<CantidadSuministro>& lista, const std::string& id) {
    int total = 0;
    for (const CantidadSuministro& suministro : lista) {
        if (suministro.id == id) {
            total += suministro.cantidad;
        }
    }
    return total;
}

void acumular(std::vector<CantidadSuministro>& lista, const CantidadSuministro& suministro) {
    for (CantidadSuministro& existente : lista) {
        if (existente.id == suministro.id) {
            existente.cantidad += suministro.cantidad;
            return;
        }
    }
    lista.push_back(suministro);
}

float distancia(Punto a, Punto b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

} // namespace

MisionBase::MisionBase(PlanificadorMapa& planificador, Combate& combate, float radio_recoleccion)
    : planificador_(planificador), combate_(combate), radio_recoleccion_(radio_recoleccion),
      estado_{}, activa_(false) {
}

void MisionBase::iniciar(const PropuestaMision& propuesta) {
    id_caso_ = propuesta.id_caso;
    resultado_.reset();

    estado_.terreno = planificador_.preparar(propuesta);
    combate_.iniciar(estado_.terreno);
    estado_.combate = combate_.estado();
    estado_.requeridos = propuesta.requeridos;
    estado_.recogidos.clear();
    estado_.segundos_restantes = static_cast<float>(std::max(propuesta.limite_segundos, 0));
    activa_ = true;
}

void MisionBase::actualizar(Punto direccion, float segundos) {
    if (!activa_) {
        return;
    }

    // Nunca se simula mas alla del tiempo que queda.
    const float paso = std::max(0.0F, std::min(segundos, estado_.segundos_restantes));

    const auto apariciones = planificador_.avanzar(paso, estado_.combate.jugador, estado_.terreno);

    for (const Aparicion& aparicion : apariciones) {
        combate_.agregar_enemigo(aparicion);
    }
    combate_.actualizar(direccion, paso);
    estado_.combate = combate_.estado();
    estado_.segundos_restantes -= paso;

    // Sin vida se resuelve antes que la recoleccion.
    if (estado_.combate.vida <= 0) {
        finalizar(DesenlaceMision::fallida, MotivoFalloMision::sin_vida);
        return;
    }

    recolectar();
    if (!activa_) {
        return;
    }

    // Recoger el ultimo suministro en el ultimo instante cuenta como exito.
    if (estado_.segundos_restantes <= 0.0F) {
        finalizar(DesenlaceMision::fallida, MotivoFalloMision::tiempo_agotado);
    }
}

const EstadoMision& MisionBase::estado() const {
    return estado_;
}

std::optional<ResultadoMision> MisionBase::extraer_resultado() {
    std::optional<ResultadoMision> entregado = std::move(resultado_);
    resultado_.reset();
    return entregado;
}

void MisionBase::suscribir(ReceptorEventosMision& receptor) {
    if (std::find(receptores_.begin(), receptores_.end(), &receptor) == receptores_.end()) {
        receptores_.push_back(&receptor);
    }
}

void MisionBase::desuscribir(ReceptorEventosMision& receptor) {
    receptores_.erase(std::remove(receptores_.begin(), receptores_.end(), &receptor),
                      receptores_.end());
}

void MisionBase::recolectar() {
    std::vector<CantidadSuministro> tomados;
    std::vector<SuministroEnMapa>& objetos = estado_.terreno.suministros;

    // Primero se actualiza el inventario y el terreno; despues se avisa.
    for (auto objeto = objetos.begin(); objeto != objetos.end();) {
        if (distancia(estado_.combate.jugador, objeto->posicion) <= radio_recoleccion_) {
            acumular(estado_.recogidos, objeto->contenido);
            tomados.push_back(objeto->contenido);
            objeto = objetos.erase(objeto);
        }

        else {
            ++objeto;
        }
    }

    const std::vector<ReceptorEventosMision*> copia = receptores_;
    for (const CantidadSuministro& tomado : tomados) {
        for (ReceptorEventosMision* receptor : copia) {
            receptor->suministro_recogido(tomado);
        }
    }

    if (objetivo_cubierto()) {
        finalizar(DesenlaceMision::completada, std::nullopt);
    }
}

bool MisionBase::objetivo_cubierto() const {
    return std::all_of(estado_.requeridos.begin(), estado_.requeridos.end(),
                       [&](const CantidadSuministro& requerido) {
                           return cantidad_de(estado_.recogidos, requerido.id) >=
                                  cantidad_de(estado_.requeridos, requerido.id);
                       });
}

void MisionBase::finalizar(DesenlaceMision desenlace, std::optional<MotivoFalloMision> motivo) {
    activa_ = false;
    resultado_ = ResultadoMision{id_caso_, desenlace, estado_.recogidos,
                                 desenlace == DesenlaceMision::fallida ? motivo : std::nullopt};

    const std::vector<ReceptorEventosMision*> copia = receptores_;
    for (ReceptorEventosMision* receptor : copia) {
        receptor->intento_finalizado(desenlace);
    }
}

} // namespace veterinaria