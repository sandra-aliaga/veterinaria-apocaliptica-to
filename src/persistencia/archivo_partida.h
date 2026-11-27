#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace veterinaria {

class ArchivoPartida {
  public:
    explicit ArchivoPartida(std::filesystem::path ruta);

    bool existe() const;
    std::optional<std::string> leer() const;
    bool guardar(const std::string& contenido) const;

  private:
    std::filesystem::path ruta_;
};

} // namespace veterinaria
