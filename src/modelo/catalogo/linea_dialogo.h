#pragma once

#include <string>

namespace veterinaria {

enum class Hablante { dueno, veterinario };

struct LineaDialogo {
    Hablante hablante;
    std::string texto;
};

} // namespace veterinaria
