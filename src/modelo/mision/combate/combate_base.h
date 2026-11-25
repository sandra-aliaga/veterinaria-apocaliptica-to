#pragma once

#include "arma/arma.h"
#include "combate.h"
#include "enemigo/enemigo.h"
#include "enemigo/fabrica_enemigos.h"
#include "veterinario_jugable.h"

#include <memory>
#include <vector>

namespace veterinaria {

/// Ajustes del combate. Unidades: pixeles del mundo y segundos.
struct ConfiguracionCombate {
    int vida_jugador = 100;
    float velocidad_jugador = 120.0F;
    float radio_jugador = 8.0F;
    float radio_enemigo = 8.0F;
    float intervalo_danio_contacto = 0.5F;
    float velocidad_proyectil = 180.0F;
    float radio_proyectil = 3.0F;
    float duracion_proyectil = 3.0F;
    /// Los pasos largos se dividen para que nada atraviese obstaculos.
    float paso_maximo_segundos = 0.05F;
};

class CombateBase final : public Combate {
  public:
    CombateBase(std::unique_ptr<FabricaEnemigos> fabrica, std::unique_ptr<Arma> arma,
                ConfiguracionCombate configuracion = {});
    ~CombateBase() override;

    void iniciar(const Terreno& terreno) override;
    void agregar_enemigo(const Aparicion& aparicion) override;
    void actualizar(Punto direccion, float segundos) override;
    const EstadoCombate& estado() const override;

  private:
    struct EnemigoActivo {
        std::unique_ptr<Enemigo> enemigo;
        float segundos_para_danio;
    };

    struct Proyectil {
        Punto posicion;
        Punto velocidad;
        int danio;
        float segundos_restantes;
    };

    void avanzar(Punto direccion, float segundos);
    void mover_jugador(Punto direccion, float segundos);
    void actualizar_enemigos(float segundos);
    void actualizar_proyectiles(float segundos);
    void atacar_con_arma();
    void retirar_eliminados();
    void publicar_estado();

    Punto resolver_movimiento(Punto desde, Punto avance, float radio) const;
    bool es_transitable(Punto centro, float radio) const;
    bool en_contacto(Punto enemigo, float alcance_extra) const;

    std::unique_ptr<FabricaEnemigos> fabrica_;
    std::unique_ptr<Arma> arma_;
    ConfiguracionCombate configuracion_;
    Terreno terreno_;
    VeterinarioJugable veterinario_;
    std::vector<EnemigoActivo> enemigos_;
    std::vector<Proyectil> proyectiles_;
    EstadoCombate estado_;
};

} // namespace veterinaria
