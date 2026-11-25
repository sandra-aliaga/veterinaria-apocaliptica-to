#include "controlador_prueba.h"

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace veterinaria {
namespace {

constexpr int version_guardado = 1;

bool entre(float valor, float minimo, float maximo) {
    return std::isfinite(valor) && valor >= minimo && valor <= maximo;
}

} // namespace

void ControladorPrueba::guardar_progreso() {
    std::ostringstream salida;
    salida.imbue(std::locale::classic());
    salida << std::setprecision(std::numeric_limits<float>::max_digits10);
    salida << version_guardado << ' ' << caso_ << ' ' << linea_ << ' ' << saldo_ << ' '
           << static_cast<int>(pantalla_) << ' ' << static_cast<int>(fase_) << ' '
           << static_cast<int>(destino_) << ' ' << opacidad_ << ' '
           << (motivo_ ? static_cast<int>(*motivo_) : -1);
    if (pantalla_ == Pantalla::mision) {
        const auto& combate = mision_.combate;
        salida << ' ' << combate.jugador.x << ' ' << combate.jugador.y << ' '
               << mision_.segundos_restantes << ' '
               << static_cast<int>(combate.movimiento_jugador.orientacion);
        for (const auto& recogido : mision_.recogidos) {
            salida << ' ' << recogido.cantidad;
        }
    }
    salida << '\n';

    if (archivo_partida_.guardar(salida.str())) {
        hay_guardado_ = true;
        mensaje_guardado_ = "Progreso guardado.";
    }

    else {
        mensaje_guardado_ = "No se pudo guardar.";
    }
}

bool ControladorPrueba::cargar_progreso() {
    const auto contenido = archivo_partida_.leer();

    if (!contenido) {
        return false;
    }
    std::istringstream entrada(*contenido);
    entrada.imbue(std::locale::classic());
    int version{};
    int saldo{};
    int num_pantalla{};
    int num_fase{};
    int num_destino{};
    int motivo{};
    std::size_t caso{};
    std::size_t linea{};
    float opacidad{};

    entrada >> version >> caso >> linea >> saldo;
    entrada >> num_pantalla >> num_fase >> num_destino >> opacidad >> motivo;

    if (!entrada || version != version_guardado || caso > casos_.size() || saldo < 0 ||
        num_fase < 0 || num_fase > static_cast<int>(FasePantalla::tratamiento) || motivo < -1 ||
        motivo > static_cast<int>(MotivoFalloMision::tiempo_agotado) || !entre(opacidad, 0, 1)) {
        return false;
    }
    const auto fase = static_cast<FasePantalla>(num_fase);
    const auto destino = static_cast<FasePantalla>(num_destino);
    const bool en_mision = fase == FasePantalla::jugando || fase == FasePantalla::regresando;
    const auto pantalla = en_mision ? Pantalla::mision : Pantalla::clinica;

    if (num_pantalla != static_cast<int>(pantalla) ||
        (caso == casos_.size()) != (fase == FasePantalla::sin_casos) ||
        (destino != FasePantalla::tratamiento && destino != FasePantalla::fallo)) {
        return false;
    }

    Punto posicion{};
    float segundos{};
    int orientacion{};
    std::vector<int> cantidades;

    if (en_mision) {
        const auto& limites = configuracion_prueba::limites_mision;
        const float x_final = limites.origen.x + limites.ancho;
        const float y_final = limites.origen.y + limites.alto;
        entrada >> posicion.x >> posicion.y >> segundos >> orientacion;

        if (!entrada || !entre(posicion.x, limites.origen.x, x_final) ||
            !entre(posicion.y, limites.origen.y, y_final) ||
            !entre(segundos, 0, configuracion_prueba::duracion_mision) || orientacion < 0 ||
            orientacion > static_cast<int>(OrientacionHorizontal::derecha)) {
            return false;
        }

        for (const auto& suministro : casos_[caso].ficha.suministros) {
            int cantidad{};

            if (!(entrada >> cantidad) || (cantidad != 0 && cantidad != suministro.cantidad)) {
                return false;
            }
            cantidades.push_back(cantidad);
        }
    }
    entrada >> std::ws;

    if (!entrada.eof()) {
        return false;
    }

    caso_ = caso;

    if (en_mision) {
        preparar_mision();
        mision_.combate.jugador = posicion;
        mision_.combate.movimiento_jugador.orientacion =
            static_cast<OrientacionHorizontal>(orientacion);
        mision_.segundos_restantes = segundos;

        for (std::size_t i = 0; i < cantidades.size(); ++i) {
            mision_.recogidos[i].cantidad = cantidades[i];
        }
        auto& objetos = mision_.terreno.suministros;

        for (std::size_t i = cantidades.size(); i > 0; --i) {
            if (cantidades[i - 1] > 0) {
                objetos.erase(objetos.begin() + static_cast<std::ptrdiff_t>(i - 1));
            }
        }
    }
    pantalla_ = pantalla;
    cambiar_fase(fase);
    const auto& dialogo = dialogo_actual();

    if (linea < dialogo.size()) {
        linea_ = linea;
    }
    destino_ = destino;
    saldo_ = saldo;
    opacidad_ = opacidad;
    motivo_ = motivo < 0 ? std::nullopt
                         : std::optional<MotivoFalloMision>{static_cast<MotivoFalloMision>(motivo)};
    return true;
}

} // namespace veterinaria
