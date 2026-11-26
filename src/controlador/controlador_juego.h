#pragma once

#include "../modelo/clinica/consultorio.h"
#include "../modelo/mision/mision.h"
#include "../persistencia/archivo_partida.h"
#include "configuracion_juego.h"
#include "controlador.h"

#include <cstddef>
#include <vector>

namespace veterinaria {

class FabricaMascotas;

/// Coordina el recorrido entre el consultorio, la mision y sus pantallas.
class ControladorJuego final : public Controlador, private ReceptorEventosMision {
  public:
    explicit ControladorJuego(
        Consultorio& consultorio, Mision& mision, const FabricaMascotas& fabrica_mascotas,
        std::vector<CasoClinico> casos,
        std::filesystem::path ruta_guardado = configuracion_juego::ruta_guardado);

    ~ControladorJuego() override;
    ControladorJuego(const ControladorJuego&) = delete;
    ControladorJuego& operator=(const ControladorJuego&) = delete;

    void actualizar(const EntradaCuadro& entrada, float segundos) override;
    EstadoPantalla estado_pantalla() const override;

    void examinar_mascota() override;
    void aceptar_caso() override;
    void rechazar_caso() override;
    void reintentar_mision() override;
    void avanzar_dialogo() override;
    void alternar_menu() override;

  private:
    void suministro_recogido(const CantidadSuministro& suministro) override;
    void intento_finalizado(DesenlaceMision desenlace) override;
    void reponer_cola(std::size_t desde, int saldo);
    void terminar_dialogo();
    const std::vector<LineaDialogo>& dialogo_actual() const;
    void cambiar_fase(FasePantalla fase);
    void siguiente_caso();
    void preparar_mision();
    void actualizar_mision(Punto direccion, float segundos);
    void actualizar_fundido(float segundos);
    void iniciar_desde_cero();
    void guardar_progreso();
    bool cargar_progreso();

    Consultorio& consultorio_;
    Mision& mision_;
    const FabricaMascotas& fabrica_mascotas_;
    ArchivoPartida archivo_partida_;
    std::vector<CasoClinico> casos_;
    std::size_t caso_ = 0;
    std::size_t linea_ = 0;
    Pantalla pantalla_ = Pantalla::inicio;
    FasePantalla fase_ = FasePantalla::portada;
    FasePantalla destino_ = FasePantalla::tratamiento;
    bool pausa_ = false;
    bool hay_guardado_ = false;
    bool confirmar_reinicio_ = false;
    bool salida_solicitada_ = false;
    std::string mensaje_guardado_;
    float opacidad_ = 0.0f;
    std::optional<PropuestaMision> propuesta_;
    bool resultado_pendiente_ = false;
    std::optional<CantidadSuministro> aviso_recogida_;
    float segundos_aviso_recogida_ = 0.0F;
    std::optional<MotivoFalloMision> motivo_;
    std::optional<double> edad_humana_mascota_;
};

} // namespace veterinaria
