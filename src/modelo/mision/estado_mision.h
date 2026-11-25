#pragma once

#include "../catalogo/cantidad_suministro.h"
#include "estado_movimiento.h"
#include "geometria_mapa.h"
#include "mapa/obstaculo_mapa.h"

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
};

enum class TipoEnemigo { animal_de_contacto, superviviente_de_contacto, superviviente_a_distancia };

struct Aparicion {
    TipoEnemigo tipo;
    Punto posicion;
};

struct EstadoEnemigo {
    TipoEnemigo tipo;
    Punto posicion;
    int vida;
    float velocidad;
    int danio;
    EstadoMovimiento movimiento;
};

struct EstadoCombate {
    Punto jugador;
    int vida;
    EstadoMovimiento movimiento_jugador;
    std::vector<EstadoEnemigo> enemigos;
    std::vector<Punto> proyectiles;
};

struct EstadoMision {
    Terreno terreno;
    EstadoCombate combate;
    std::vector<CantidadSuministro> requeridos;
    std::vector<CantidadSuministro> recogidos;
    float segundos_restantes;
};

} // namespace veterinaria
