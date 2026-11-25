#pragma once

#include "vista.h"

#include <memory>

namespace veterinaria {

std::unique_ptr<Vista> crear_vista_raylib();

} // namespace veterinaria
