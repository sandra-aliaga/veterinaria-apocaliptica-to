#pragma once

namespace veterinaria {

enum class OrientacionHorizontal { izquierda, derecha };

struct EstadoMovimiento {
    OrientacionHorizontal orientacion;
    bool en_movimiento;
};

} // namespace veterinaria
