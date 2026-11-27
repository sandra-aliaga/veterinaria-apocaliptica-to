#pragma once

#include "../modelo/mision/geometria_mapa.h"

namespace veterinaria::configuracion_juego {

inline constexpr char directorio_casos[] = "datos";
inline constexpr char ruta_guardado[] = "datos/progreso_previa.txt";
inline constexpr Rectangulo limites_mision{{0, 0}, 3072, 2048};
inline constexpr float radio_recogida = 15.0F;
inline constexpr float velocidad_fundido = 2.5F;
inline constexpr int danio_arma = 32;
inline constexpr float recarga_arma = 0.65F;
inline constexpr float alcance_arma = 108.0F;
inline constexpr float duracion_aviso_recogida = 2.0F;

} // namespace veterinaria::configuracion_juego
