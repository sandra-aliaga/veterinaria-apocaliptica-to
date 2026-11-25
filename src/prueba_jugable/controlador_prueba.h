#pragma once

#include "../controlador/controlador.h"
#include "../persistencia/archivo_partida.h"
#include "configuracion_prueba.h"

#include <array>
#include <cstddef>

namespace veterinaria {

/// Controlador provisional para recorrer las pantallas con datos de ejemplo.
class ControladorPrueba final : public Controlador {
  public:
    explicit ControladorPrueba(
        std::filesystem::path ruta_guardado = configuracion_prueba::ruta_guardado);

    void actualizar(const EntradaCuadro& entrada, float segundos) override;
    EstadoPantalla estado_pantalla() const override;

    void examinar_mascota() override;
    void aceptar_caso() override;
    void rechazar_caso() override;
    void reintentar_mision() override;
    void avanzar_dialogo() override;
    void alternar_menu() override;

  private:
    struct CasoPrueba {
        CasoClinico ficha;
        const char* retrato_tratamiento;
    };

    static std::array<CasoPrueba, 2> fichas_de_prueba();
    const std::vector<LineaDialogo>& dialogo_actual() const;
    void cambiar_fase(FasePantalla fase);
    void siguiente_caso();
    void preparar_mision();
    void actualizar_mision(Punto direccion, float segundos);
    void mover_jugador(Punto direccion, float segundos);
    void recoger_suministros();
    void volver_a_clinica(FasePantalla fase, std::optional<MotivoFalloMision> motivo);
    void actualizar_fundido(float segundos);
    void iniciar_desde_cero();
    void guardar_progreso();
    bool cargar_progreso();

    ArchivoPartida archivo_partida_;
    std::array<CasoPrueba, 2> casos_;
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
    int saldo_ = 0;
    float opacidad_ = 0.0f;
    EstadoMision mision_{};
    std::optional<MotivoFalloMision> motivo_;
};

} // namespace veterinaria
