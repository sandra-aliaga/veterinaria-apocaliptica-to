#include "controlador_juego.h"

#include "../modelo/catalogo/mascota/fabrica_mascotas.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace veterinaria {

ControladorJuego::ControladorJuego(Consultorio& consultorio, Mision& mision,
                                   const FabricaMascotas& fabrica_mascotas,
                                   std::vector<CasoClinico> casos,
                                   std::filesystem::path ruta_guardado)
    : consultorio_(consultorio), mision_(mision), fabrica_mascotas_(fabrica_mascotas),
      archivo_partida_(std::move(ruta_guardado)), casos_(std::move(casos)),
      hay_guardado_(archivo_partida_.existe()) {
    mision_.suscribir(*this);
}

ControladorJuego::~ControladorJuego() {
    mision_.desuscribir(*this);
}

EstadoPantalla ControladorJuego::estado_pantalla() const {
    EstadoPantalla estado{};
    estado.pantalla = pantalla_;
    estado.fase = fase_;
    estado.menu_abierto = pausa_;
    estado.opacidad_fundido = opacidad_;
    estado.saldo = consultorio_.saldo();
    estado.edad_humana_mascota = edad_humana_mascota_;
    estado.motivo_fallo = motivo_;
    estado.mensaje_guardado = mensaje_guardado_;
    estado.confirmar_reinicio = confirmar_reinicio_;
    estado.salida_solicitada = salida_solicitada_;

    if (pantalla_ == Pantalla::inicio) {
        estado.acciones_disponibles = {Accion::iniciar_partida};

        if (hay_guardado_) {
            estado.acciones_disponibles.push_back(Accion::nueva_partida);
        }
        return estado;
    }

    estado.acciones_disponibles = {Accion::alternar_menu};

    if (confirmar_reinicio_) {
        estado.acciones_disponibles.push_back(Accion::confirmar_reinicio);
    }

    else if (pausa_) {
        estado.acciones_disponibles.insert(
            estado.acciones_disponibles.end(),
            {Accion::guardar_progreso, Accion::reiniciar_partida, Accion::salir_partida});
    }

    if (pantalla_ == Pantalla::mision) {
        estado.mision = &mision_.estado();
        estado.aviso_recogida = aviso_recogida_ ? &*aviso_recogida_ : nullptr;
        return estado;
    }

    if (caso_ < casos_.size()) {
        estado.ficha = &casos_[caso_];
        const auto& dialogo = dialogo_actual();

        if (linea_ < dialogo.size()) {
            estado.dialogo = &dialogo[linea_];
        }
    }

    if (pausa_ || opacidad_ > 0.0f) {
        return estado;
    }

    switch (fase_) {
    case FasePantalla::por_examinar:
        estado.acciones_disponibles.push_back(Accion::examinar);
        break;
    case FasePantalla::resultado_examen:
        estado.acciones_disponibles.push_back(Accion::aceptar);
        estado.acciones_disponibles.push_back(Accion::rechazar);
        break;
    case FasePantalla::reintento:
        estado.acciones_disponibles.push_back(Accion::reintentar);
        break;
    default:
        if (estado.dialogo) {
            estado.acciones_disponibles.push_back(Accion::avanzar_dialogo);
        }
        break;
    }
    return estado;
}

void ControladorJuego::actualizar(const EntradaCuadro& entrada, float segundos) {
    if (entrada.salir) {
        salida_solicitada_ = true;
        return;
    }

    if (entrada.accion) {
        const auto acciones = estado_pantalla().acciones_disponibles;

        if (std::find(acciones.begin(), acciones.end(), *entrada.accion) != acciones.end()) {
            switch (*entrada.accion) {
            case Accion::iniciar_partida:
                if (!cargar_progreso()) {
                    iniciar_desde_cero();

                    if (hay_guardado_) {
                        mensaje_guardado_ = "Guardado incompatible.";
                    }
                }
                break;
            case Accion::nueva_partida:
            case Accion::confirmar_reinicio:
                iniciar_desde_cero();
                break;
            case Accion::guardar_progreso:
                guardar_progreso();
                break;
            case Accion::reiniciar_partida:
                confirmar_reinicio_ = true;
                break;
            case Accion::salir_partida:
                salida_solicitada_ = true;
                break;
            case Accion::examinar:
                examinar_mascota();
                break;
            case Accion::aceptar:
                aceptar_caso();
                break;
            case Accion::rechazar:
                rechazar_caso();
                break;
            case Accion::reintentar:
                reintentar_mision();
                break;
            case Accion::avanzar_dialogo:
                avanzar_dialogo();
                break;
            case Accion::alternar_menu:
                alternar_menu();
                break;
            }
        }
    }

    if (pausa_ || salida_solicitada_) {
        return;
    }

    if (fase_ == FasePantalla::regresando || opacidad_ > 0.0f) {
        actualizar_fundido(segundos);
    }

    else if (fase_ == FasePantalla::jugando) {
        actualizar_mision(entrada.direccion, segundos);
    }
}

void ControladorJuego::examinar_mascota() {
    if (const auto* ficha = consultorio_.ficha_pendiente()) {
        const auto mascota = fabrica_mascotas_.crear(ficha->mascota);
        edad_humana_mascota_ = mascota->calcularEdadHumana();
        cambiar_fase(FasePantalla::resultado_examen);
    }
}

void ControladorJuego::aceptar_caso() {
    auto propuesta = consultorio_.aceptar();

    if (propuesta) {
        propuesta_ = std::move(propuesta);
        cambiar_fase(FasePantalla::aceptacion);
    }
}

void ControladorJuego::rechazar_caso() {
    if (consultorio_.rechazar()) {
        cambiar_fase(FasePantalla::rechazo);
    }
}

void ControladorJuego::reintentar_mision() {
    propuesta_ = consultorio_.reintentar();
    preparar_mision();
}

void ControladorJuego::alternar_menu() {
    if (confirmar_reinicio_) {
        confirmar_reinicio_ = false;
        return;
    }
    pausa_ = !pausa_;
}

void ControladorJuego::reponer_cola(std::size_t desde, int saldo) {
    consultorio_.reiniciar(saldo);

    for (std::size_t i = desde; i < casos_.size(); ++i) {
        consultorio_.recibir(casos_[i]);
    }
}

void ControladorJuego::iniciar_desde_cero() {
    caso_ = 0;
    reponer_cola(caso_, 0);
    opacidad_ = 0.0F;
    propuesta_.reset();
    resultado_pendiente_ = false;
    aviso_recogida_.reset();
    edad_humana_mascota_.reset();
    motivo_.reset();
    pausa_ = false;
    confirmar_reinicio_ = false;
    mensaje_guardado_.clear();
    pantalla_ = Pantalla::clinica;
    destino_ = FasePantalla::tratamiento;
    cambiar_fase(casos_.empty() ? FasePantalla::sin_casos : FasePantalla::llegada);
}

void ControladorJuego::avanzar_dialogo() {
    if (++linea_ < dialogo_actual().size()) {
        return;
    }

    terminar_dialogo();
}

void ControladorJuego::terminar_dialogo() {
    switch (fase_) {
    case FasePantalla::llegada:
        cambiar_fase(FasePantalla::por_examinar);
        break;
    case FasePantalla::aceptacion:
        preparar_mision();
        break;
    case FasePantalla::rechazo:
    case FasePantalla::tratamiento:
        siguiente_caso();
        break;
    case FasePantalla::fallo:
        cambiar_fase(FasePantalla::reintento);
        break;
    default:
        break;
    }
}

const std::vector<LineaDialogo>& ControladorJuego::dialogo_actual() const {
    static const std::vector<LineaDialogo> sin_dialogo;

    if (caso_ >= casos_.size()) {
        return sin_dialogo;
    }
    const auto& dialogos = casos_[caso_].dialogos;

    switch (fase_) {
    case FasePantalla::llegada:
        return dialogos.llegada;
    case FasePantalla::aceptacion:
        return dialogos.aceptacion;
    case FasePantalla::rechazo:
        return dialogos.rechazo;
    case FasePantalla::fallo:
        return dialogos.fallo;
    case FasePantalla::tratamiento:
        return dialogos.tratamiento;
    default:
        return sin_dialogo;
    }
}

void ControladorJuego::cambiar_fase(FasePantalla fase) {
    fase_ = fase;
    linea_ = 0;

    if (dialogo_actual().empty()) {
        terminar_dialogo();
    }
}

void ControladorJuego::siguiente_caso() {
    motivo_.reset();
    edad_humana_mascota_.reset();
    ++caso_;
    cambiar_fase(consultorio_.ficha_pendiente() ? FasePantalla::llegada : FasePantalla::sin_casos);
}

void ControladorJuego::preparar_mision() {
    if (!propuesta_) {
        return;
    }

    mision_.iniciar(*propuesta_);
    propuesta_.reset();
    motivo_.reset();
    aviso_recogida_.reset();
    resultado_pendiente_ = false;
    pantalla_ = Pantalla::mision;
    cambiar_fase(FasePantalla::jugando);
}

void ControladorJuego::actualizar_mision(Punto direccion, float segundos) {
    if (aviso_recogida_) {
        segundos_aviso_recogida_ -= segundos;

        if (segundos_aviso_recogida_ <= 0.0F) {
            aviso_recogida_.reset();
        }
    }

    mision_.actualizar(direccion, segundos);

    if (!resultado_pendiente_) {
        return;
    }

    resultado_pendiente_ = false;
    const auto resultado = mision_.extraer_resultado();

    if (!resultado) {
        throw std::logic_error("La misión terminó sin publicar un resultado.");
    }

    const auto registro = consultorio_.registrar_resultado(*resultado);

    if (registro != RegistroMision::tratado && registro != RegistroMision::fallo_registrado) {
        throw std::logic_error("El consultorio rechazó el resultado de la misión.");
    }

    motivo_ = resultado->motivo_fallo;
    destino_ =
        registro == RegistroMision::tratado ? FasePantalla::tratamiento : FasePantalla::fallo;
    cambiar_fase(FasePantalla::regresando);
}

void ControladorJuego::suministro_recogido(const CantidadSuministro& suministro) {
    aviso_recogida_ = suministro;
    segundos_aviso_recogida_ = configuracion_juego::duracion_aviso_recogida;
}

void ControladorJuego::intento_finalizado(DesenlaceMision) {
    resultado_pendiente_ = true;
}

void ControladorJuego::actualizar_fundido(float segundos) {
    const float paso = segundos * configuracion_juego::velocidad_fundido;

    if (fase_ == FasePantalla::regresando) {
        opacidad_ = std::min(1.0f, opacidad_ + paso);

        if (opacidad_ == 1.0f) {
            pantalla_ = Pantalla::clinica;

            cambiar_fase(destino_);
        }
    }

    else {
        opacidad_ = std::max(0.0f, opacidad_ - paso);
    }
}

} // namespace veterinaria
