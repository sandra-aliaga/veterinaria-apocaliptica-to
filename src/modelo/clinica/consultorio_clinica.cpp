#include "consultorio_clinica.h"

#include <map>
#include <string>
#include <utility>

namespace veterinaria {

namespace {

/// Un exito solo es valido si lo recogido cubre cada cantidad requerida del caso.
bool cubre_requeridos(const std::vector<CantidadSuministro>& requeridos,
                      const std::vector<CantidadSuministro>& recogidos) {
    std::map<std::string, long long> disponibles;
    for (const CantidadSuministro& suministro : recogidos) {
        if (suministro.cantidad > 0) {
            disponibles[suministro.id] += suministro.cantidad;
        }
    }

    for (const CantidadSuministro& requerido : requeridos) {
        const auto encontrado = disponibles.find(requerido.id);
        const long long tengo = encontrado == disponibles.end() ? 0 : encontrado->second;
        if (tengo < requerido.cantidad) {
            return false;
        }
    }
    return true;
}

} // namespace

ConsultorioClinica::ConsultorioClinica() : intento_en_curso_(false), saldo_(0) {
}

ConsultorioClinica::~ConsultorioClinica() = default;

void ConsultorioClinica::recibir(CasoClinico caso) {
    cola_.encolar(std::move(caso));
}

const CasoClinico* ConsultorioClinica::ficha_pendiente() const {
    return cola_.frente();
}

const CasoClinico* ConsultorioClinica::caso_activo() const {
    return activo_ ? &*activo_ : nullptr;
}

PropuestaMision ConsultorioClinica::proponer_mision() const {
    PropuestaMision propuesta;
    propuesta.id_caso = activo_->id;
    propuesta.requeridos = activo_->suministros;
    propuesta.limite_segundos = 300;
    return propuesta;
}

std::optional<PropuestaMision> ConsultorioClinica::aceptar() {
    if (activo_ || cola_.frente() == nullptr) {
        return std::nullopt;
    }
    activo_ = cola_.extraer();
    intento_en_curso_ = true;
    return proponer_mision();
}

bool ConsultorioClinica::rechazar() {
    if (activo_ || cola_.frente() == nullptr) {
        return false;
    }
    cola_.extraer();
    return true;
}

std::optional<PropuestaMision> ConsultorioClinica::reintentar() {
    // Solo se reintenta tras un fallo ya registrado; no hay dos intentos a la vez.
    if (!activo_ || intento_en_curso_) {
        return std::nullopt;
    }
    intento_en_curso_ = true;
    return proponer_mision();
}

RegistroMision ConsultorioClinica::registrar_resultado(const ResultadoMision& resultado) {
    if (!activo_) {
        return RegistroMision::sin_caso_activo;
    }

    if (!intento_en_curso_ || resultado.id_caso != activo_->id) {
        return RegistroMision::resultado_invalido;
    }

    if (resultado.desenlace == DesenlaceMision::completada) {
        if (!cubre_requeridos(activo_->suministros, resultado.recogidos)) {
            return RegistroMision::resultado_invalido;
        }
        saldo_ += activo_->recompensa;
        activo_.reset();
        intento_en_curso_ = false;
        return RegistroMision::tratado;
    }

    // Un fallo debe indicar si fue por falta de vida o de tiempo.
    if (!resultado.motivo_fallo.has_value()) {
        return RegistroMision::resultado_invalido;
    }
    intento_en_curso_ = false;
    return RegistroMision::fallo_registrado;
}

int ConsultorioClinica::saldo() const {
    return saldo_;
}

} // namespace veterinaria
