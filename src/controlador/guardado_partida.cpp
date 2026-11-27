#include "controlador_juego.h"

#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace veterinaria {
namespace {

constexpr int version_anterior = 1;
constexpr int version_guardado = 2;
constexpr int sin_motivo = -1;
constexpr std::array<std::string_view, 2> casos_de_la_previa{"caso_mora", "caso_toby"};

} // namespace

void ControladorJuego::guardar_progreso() {
    std::ostringstream salida;
    const auto fase = fase_ == FasePantalla::regresando ? destino_ : fase_;
    const std::string id = caso_ < casos_.size() ? casos_[caso_].id : "";
    const int motivo = motivo_ ? static_cast<int>(*motivo_) : sin_motivo;
    salida << version_guardado << ' ' << std::quoted(id) << ' ' << linea_ << ' '
           << consultorio_.saldo() << ' ' << static_cast<int>(fase) << ' ' << motivo << '\n';

    if (archivo_partida_.guardar(salida.str())) {
        hay_guardado_ = true;
        mensaje_guardado_ = fase == FasePantalla::jugando ? "Guardado. El intento se reiniciará."
                                                          : "Progreso guardado.";
    }

    else {
        mensaje_guardado_ = "No se pudo guardar.";
    }
}

bool ControladorJuego::cargar_progreso() {
    const auto contenido = archivo_partida_.leer();

    if (!contenido) {
        return false;
    }

    std::istringstream entrada(*contenido);
    int version = 0;
    int saldo = 0;
    int num_fase = 0;
    int motivo = sin_motivo;
    std::size_t linea = 0;
    std::string id;
    entrada >> version;

    if (version == version_guardado) {
        entrada >> std::quoted(id) >> linea >> saldo >> num_fase >> motivo;

        if (!entrada) {
            return false;
        }

        entrada >> std::ws;

        if (!entrada.eof()) {
            return false;
        }
    }

    else if (version == version_anterior) {
        std::size_t indice = 0;
        int pantalla_anterior = 0;
        int destino_anterior = 0;
        float opacidad_anterior = 0.0F;
        entrada >> indice >> linea >> saldo >> pantalla_anterior >> num_fase >> destino_anterior >>
            opacidad_anterior >> motivo;

        if (!entrada || indice > casos_de_la_previa.size()) {
            return false;
        }

        if (indice < casos_de_la_previa.size()) {
            id = casos_de_la_previa[indice];
        }

        if (num_fase == static_cast<int>(FasePantalla::regresando)) {
            num_fase = static_cast<int>(FasePantalla::jugando);
            motivo = sin_motivo;
        }
    }

    else {
        return false;
    }

    if (saldo < 0 || num_fase < 0 || num_fase > static_cast<int>(FasePantalla::tratamiento) ||
        motivo < sin_motivo || motivo > static_cast<int>(MotivoFalloMision::tiempo_agotado)) {
        return false;
    }

    const auto fase = static_cast<FasePantalla>(num_fase);
    const bool caso_cerrado = fase == FasePantalla::rechazo || fase == FasePantalla::tratamiento;
    const bool intento_fallido = fase == FasePantalla::fallo || fase == FasePantalla::reintento;
    const auto ficha = std::find_if(casos_.begin(), casos_.end(),
                                    [&id](const CasoClinico& caso) { return caso.id == id; });

    if ((id.empty() != (fase == FasePantalla::sin_casos)) ||
        (!id.empty() && ficha == casos_.end()) || (intento_fallido != (motivo != sin_motivo)) ||
        fase == FasePantalla::regresando) {
        return false;
    }

    caso_ = static_cast<std::size_t>(ficha - casos_.begin());
    reponer_cola(caso_cerrado ? caso_ + 1 : caso_, saldo);
    pantalla_ = Pantalla::clinica;
    propuesta_.reset();

    if (fase == FasePantalla::aceptacion || fase == FasePantalla::jugando || intento_fallido) {
        propuesta_ = consultorio_.aceptar();
    }

    if (intento_fallido) {
        motivo_ = static_cast<MotivoFalloMision>(motivo);
        consultorio_.registrar_resultado({id, DesenlaceMision::fallida, {}, motivo_});
        propuesta_.reset();
    }

    if (fase == FasePantalla::jugando) {
        preparar_mision();
        mensaje_guardado_ = "Has vuelto al inicio del intento.";
        return true;
    }

    if (fase == FasePantalla::resultado_examen) {
        examinar_mascota();
        return true;
    }

    cambiar_fase(fase);

    if (fase_ == fase && linea < dialogo_actual().size()) {
        linea_ = linea;
    }

    return true;
}

} // namespace veterinaria
