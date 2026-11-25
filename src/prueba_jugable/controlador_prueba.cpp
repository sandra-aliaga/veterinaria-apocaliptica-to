#include "controlador_prueba.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace veterinaria {
namespace {

bool dentro(Punto punto, const Rectangulo& area) {
    return punto.x >= area.origen.x && punto.x <= area.origen.x + area.ancho &&
           punto.y >= area.origen.y && punto.y <= area.origen.y + area.alto;
}

} // namespace

std::array<ControladorPrueba::CasoPrueba, 2> ControladorPrueba::fichas_de_prueba() {
    return {{
        {
            {
                "mora",
                {"mora", "Mora", "Gata", 24, "arte/mascotas/mora.png"},
                {"elena", "Elena", "arte/duenos/elena.png"},
                {
                    {
                        {Hablante::dueno, "Mora volvió del patio sin apoyar la pata."},
                        {Hablante::veterinario, "Déjala sobre la mesa. Voy a revisarla."},
                    },
                    {{Hablante::veterinario, "Necesito vendas y antiséptico. Voy a buscarlos."}},
                    {{Hablante::dueno, "Entiendo. Buscaré otro lugar para atenderla."}},
                    {{Hablante::veterinario, "No conseguí los suministros a tiempo."}},
                    {{Hablante::dueno, "Ya apoya un poco mejor la pata. Gracias."}},
                },
                {{"vendas", 2}, {"antiseptico", 1}},
                20,
            },
            "arte/mascotas/mora_relajada.png",
        },
        {
            {
                "toby",
                {"toby", "Toby", "Perro", 48, "arte/mascotas/toby.png"},
                {"dueno_toby", "Dueño de Toby", "arte/duenos/dueno_toby.png"},
                {
                    {{Hablante::dueno, "Toby no ha levantado la cabeza desde ayer."}},
                    {{Hablante::veterinario, "Quédate con él. Buscaré el suero."}},
                    {{Hablante::dueno, "Me lo llevaré de vuelta al refugio."}},
                    {{Hablante::veterinario, "Aún podemos volver a buscar el suero."}},
                    {{Hablante::dueno, "Abrió los ojos en cuanto le hablé."}},
                },
                {{"suero", 1}},
                25,
            },
            "arte/mascotas/toby_recuperado.png",
        },
    }};
}

ControladorPrueba::ControladorPrueba(std::filesystem::path ruta_guardado)
    : archivo_partida_(std::move(ruta_guardado)), casos_(fichas_de_prueba()),
      hay_guardado_(archivo_partida_.existe()) {
}

EstadoPantalla ControladorPrueba::estado_pantalla() const {
    EstadoPantalla estado{};
    estado.pantalla = pantalla_;
    estado.fase = fase_;
    estado.menu_abierto = pausa_;
    estado.opacidad_fundido = opacidad_;
    estado.saldo = saldo_;
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
        estado.mision = &mision_;
        return estado;
    }

    if (caso_ < casos_.size()) {
        const auto& caso = casos_[caso_];
        estado.ficha = &caso.ficha;

        if (fase_ == FasePantalla::tratamiento) {
            estado.retrato_alternativo = caso.retrato_tratamiento;
        }
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

void ControladorPrueba::actualizar(const EntradaCuadro& entrada, float segundos) {
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

void ControladorPrueba::examinar_mascota() {
    cambiar_fase(FasePantalla::resultado_examen);
}

void ControladorPrueba::aceptar_caso() {
    cambiar_fase(FasePantalla::aceptacion);
}

void ControladorPrueba::rechazar_caso() {
    cambiar_fase(FasePantalla::rechazo);
}

void ControladorPrueba::reintentar_mision() {
    preparar_mision();
}

void ControladorPrueba::alternar_menu() {
    if (confirmar_reinicio_) {
        confirmar_reinicio_ = false;
        return;
    }
    pausa_ = !pausa_;
}

void ControladorPrueba::iniciar_desde_cero() {
    caso_ = 0;
    saldo_ = 0;
    opacidad_ = 0.0f;
    mision_ = {};
    motivo_.reset();
    pausa_ = false;
    confirmar_reinicio_ = false;
    mensaje_guardado_.clear();
    pantalla_ = Pantalla::clinica;
    destino_ = FasePantalla::tratamiento;
    cambiar_fase(FasePantalla::llegada);
}

void ControladorPrueba::avanzar_dialogo() {
    if (++linea_ < dialogo_actual().size()) {
        return;
    }

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

const std::vector<LineaDialogo>& ControladorPrueba::dialogo_actual() const {
    static const std::vector<LineaDialogo> sin_dialogo;

    if (caso_ >= casos_.size()) {
        return sin_dialogo;
    }
    const auto& dialogos = casos_[caso_].ficha.dialogos;

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

void ControladorPrueba::cambiar_fase(FasePantalla fase) {
    fase_ = fase;
    linea_ = 0;
}

void ControladorPrueba::siguiente_caso() {
    motivo_.reset();
    ++caso_;
    cambiar_fase(caso_ < casos_.size() ? FasePantalla::llegada : FasePantalla::sin_casos);
}

void ControladorPrueba::preparar_mision() {
    namespace ajustes = configuracion_prueba;

    mision_ = {};
    mision_.terreno.limites = ajustes::limites_mision;
    mision_.terreno.inicio = ajustes::inicio_jugador;
    mision_.terreno.obstaculos.assign(ajustes::obstaculos.begin(), ajustes::obstaculos.end());
    mision_.combate.jugador = mision_.terreno.inicio;
    mision_.combate.vida_maxima = ajustes::vida_jugador;
    mision_.combate.vida = mision_.combate.vida_maxima;
    mision_.combate.movimiento_jugador = {OrientacionHorizontal::derecha, false};
    mision_.requeridos = casos_[caso_].ficha.suministros;
    Punto posicion = ajustes::primer_suministro;

    for (const auto& suministro : mision_.requeridos) {
        mision_.terreno.suministros.push_back({suministro, posicion});
        mision_.recogidos.push_back({suministro.id, 0});
        posicion.x += ajustes::distancia_entre_suministros;
    }

    mision_.segundos_restantes = ajustes::duracion_mision;
    motivo_.reset();
    pantalla_ = Pantalla::mision;
    cambiar_fase(FasePantalla::jugando);
}

void ControladorPrueba::actualizar_mision(Punto direccion, float segundos) {
    mision_.segundos_restantes = std::max(0.0f, mision_.segundos_restantes - segundos);

    if (mision_.segundos_restantes == 0.0f) {
        volver_a_clinica(FasePantalla::fallo, MotivoFalloMision::tiempo_agotado);
        return;
    }

    mover_jugador(direccion, segundos);
    recoger_suministros();

    if (mision_.terreno.suministros.empty()) {
        volver_a_clinica(FasePantalla::tratamiento, std::nullopt);
    }
}

void ControladorPrueba::mover_jugador(Punto direccion, float segundos) {
    auto& combate = mision_.combate;
    const float longitud = std::hypot(direccion.x, direccion.y);
    combate.movimiento_jugador.en_movimiento = longitud > 0.0f;

    if (direccion.x != 0.0f) {
        combate.movimiento_jugador.orientacion =
            direccion.x < 0.0f ? OrientacionHorizontal::izquierda : OrientacionHorizontal::derecha;
    }

    if (longitud == 0.0f) {
        return;
    }

    const float tiempo_paso = std::min(segundos, configuracion_prueba::paso_maximo_segundos);
    const float paso = configuracion_prueba::velocidad_jugador * tiempo_paso / longitud;
    const auto es_transitable = [this](Punto punto) {
        if (!dentro(punto, mision_.terreno.limites)) {
            return false;
        }

        const auto& obstaculos = mision_.terreno.obstaculos;
        return std::none_of(obstaculos.begin(), obstaculos.end(), [punto](const auto& obstaculo) {
            return dentro(punto, obstaculo.colision);
        });
    };

    Punto candidato = combate.jugador;
    candidato.x += direccion.x * paso;

    if (es_transitable(candidato)) {
        combate.jugador.x = candidato.x;
    }

    candidato = combate.jugador;
    candidato.y += direccion.y * paso;

    if (es_transitable(candidato)) {
        combate.jugador.y = candidato.y;
    }
}

void ControladorPrueba::recoger_suministros() {
    auto& objetos = mision_.terreno.suministros;
    auto& recogidos = mision_.recogidos;
    const auto posicion = mision_.combate.jugador;

    const auto recoger_si_toca = [&](const SuministroEnMapa& objeto) {
        const float distancia_x = posicion.x - objeto.posicion.x;
        const float distancia_y = posicion.y - objeto.posicion.y;
        const float distancia = std::hypot(distancia_x, distancia_y);

        if (distancia > configuracion_prueba::radio_recogida) {
            return false;
        }

        const auto coincide = [&](const CantidadSuministro& cantidad) {
            return cantidad.id == objeto.contenido.id;
        };
        const auto recogido = std::find_if(recogidos.begin(), recogidos.end(), coincide);

        if (recogido != recogidos.end()) {
            recogido->cantidad += objeto.contenido.cantidad;
        }

        return true;
    };

    const auto primer_recogido = std::remove_if(objetos.begin(), objetos.end(), recoger_si_toca);
    objetos.erase(primer_recogido, objetos.end());
}

void ControladorPrueba::volver_a_clinica(FasePantalla fase,
                                         std::optional<MotivoFalloMision> motivo) {
    destino_ = fase;
    motivo_ = motivo;
    cambiar_fase(FasePantalla::regresando);
}

void ControladorPrueba::actualizar_fundido(float segundos) {
    const float paso = segundos * configuracion_prueba::velocidad_fundido;

    if (fase_ == FasePantalla::regresando) {
        opacidad_ = std::min(1.0f, opacidad_ + paso);

        if (opacidad_ == 1.0f) {
            pantalla_ = Pantalla::clinica;
            cambiar_fase(destino_);

            if (destino_ == FasePantalla::tratamiento) {
                saldo_ += casos_[caso_].ficha.recompensa;
            }
        }
    }

    else {
        opacidad_ = std::max(0.0f, opacidad_ - paso);
    }
}

} // namespace veterinaria
