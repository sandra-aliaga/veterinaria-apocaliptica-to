#pragma once

#include "../modelo/mision/mapa/obstaculo_mapa.h"

#include <array>

namespace veterinaria::configuracion_prueba {

inline constexpr char ruta_guardado[] = "datos/progreso_previa.txt";
inline constexpr Rectangulo limites_mision{{0, 0}, 1280, 900};
inline constexpr Punto inicio_jugador{240, 200};
inline constexpr int vida_jugador = 100;
inline constexpr float duracion_mision = 300.0f;
inline constexpr float velocidad_jugador = 120.0f;
inline constexpr float paso_maximo_segundos = 0.05f;
inline constexpr float radio_recogida = 15.0f;
inline constexpr float velocidad_fundido = 2.5f;
inline constexpr Punto primer_suministro{330, 180};
inline constexpr float distancia_entre_suministros = 85.0f;

inline constexpr std::array obstaculos{
    ObstaculoMapa{TipoObstaculo::arbol, {330, 260}, {{317, 249}, 26, 23}},
    ObstaculoMapa{TipoObstaculo::roca, {260, 335}, {{244, 324}, 32, 23}},
    ObstaculoMapa{TipoObstaculo::tronco, {403, 317}, {{382, 308}, 43, 16}},
};

} // namespace veterinaria::configuracion_prueba
