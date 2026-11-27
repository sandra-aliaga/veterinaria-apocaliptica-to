#pragma once

#include "../catalogo/cantidad_suministro.h"
#include "estado_movimiento.h"
#include "geometria_mapa.h"
#include "mapa/obstaculo_mapa.h"
#include "mapa/suelo_mapa.h"

#include <optional>
#include <vector>

namespace veterinaria {

struct SuministroEnMapa {
    CantidadSuministro contenido;
    Punto posicion;
};

struct Terreno {
    Rectangulo limites;
    Punto inicio;
    std::vector<ObstaculoMapa> obstaculos;
    std::vector<SuministroEnMapa> suministros;
    SueloMapa suelo;
};

enum class TipoEnemigo { animal_de_contacto, superviviente_de_contacto, superviviente_a_distancia };

struct Aparicion {
    TipoEnemigo tipo;
    Punto posicion;
};

struct GestoCombate {
    OrientacionHorizontal orientacion;
    float instante;
};

struct EstadoEnemigo {
    TipoEnemigo tipo;
    Punto posicion;
    int vida;
    float velocidad;
    int danio;
    EstadoMovimiento movimiento;
    bool variante_visual = false;
    std::optional<GestoCombate> disparo{};
};

struct EstadoProyectil {
    Punto posicion;
    Punto velocidad;
};

struct GolpeArma {
    Punto impacto;
    GestoCombate gesto;
};

struct EstadoCombate {
    Punto jugador;
    int vida;
    EstadoMovimiento movimiento_jugador;
    std::vector<EstadoEnemigo> enemigos;
    std::vector<EstadoProyectil> proyectiles;
    int vida_maxima = 100;
    float segundos_combate = 0.0F;
    std::optional<float> instante_danio{};
    std::optional<GolpeArma> ultimo_golpe{};
};

struct EstadoMision {
    Terreno terreno;
    EstadoCombate combate;
    std::vector<CantidadSuministro> requeridos;
    std::vector<CantidadSuministro> recogidos;
    float segundos_restantes;
};

} // namespace veterinaria
