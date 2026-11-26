#pragma once

#include <string>

namespace veterinaria {

enum class Hablante { dueno, veterinario };

struct LineaDialogo {
    Hablante hablante;
    std::string texto;
    std::string sprite_dueno{};
    std::string retrato_mascota{};
};

} // namespace veterinaria
