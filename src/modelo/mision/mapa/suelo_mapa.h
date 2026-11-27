#pragma once

#include <cstddef>
#include <vector>

namespace veterinaria {

enum class TipoPiezaSuelo { pasto, tierra, transicion };

/// Una pieza cuadrada del suelo. La vista elige el PNG por `tipo`.
struct PiezaSuelo {
    TipoPiezaSuelo tipo = TipoPiezaSuelo::pasto;

    /// Solo para `transicion`. El PNG original tiene el pasto arriba y la tierra abajo.
    /// Este valor son los giros de 90 grados en sentido horario que hay que dar a la imagen
    /// para que la tierra quede del lado correcto:
    /// 0 = tierra abajo, 1 = tierra a la izquierda, 2 = tierra arriba, 3 = tierra a la derecha.
    int giro_cuartos = 0;
};

/// Cuadricula de piezas que cubre `Terreno::limites`, fila por fila desde arriba a la izquierda.
/// La pieza (columna, fila) empieza en `limites.origen + (columna, fila) * tamano_pieza`.
/// Si `piezas` esta vacio, la vista puede usar pasto en todo el campo.
struct SueloMapa {
    float tamano_pieza = 16.0F;
    int columnas = 0;
    int filas = 0;
    std::vector<PiezaSuelo> piezas;

    /// Devuelve pasto si la posicion cae fuera de la cuadricula.
    PiezaSuelo pieza(int columna, int fila) const {
        if (columna < 0 || columna >= columnas || fila < 0 || fila >= filas) {
            return {};
        }
        const std::size_t indice =
            static_cast<std::size_t>(fila) * static_cast<std::size_t>(columnas) +
            static_cast<std::size_t>(columna);
        return indice < piezas.size() ? piezas[indice] : PiezaSuelo{};
    }
};

} // namespace veterinaria
