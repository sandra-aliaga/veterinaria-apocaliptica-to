#include "archivo_partida.h"

#include <chrono>
#include <fstream>
#include <iterator>
#include <system_error>
#include <utility>

namespace veterinaria {

ArchivoPartida::ArchivoPartida(std::filesystem::path ruta) : ruta_(std::move(ruta)) {
}

bool ArchivoPartida::existe() const {
    std::error_code error;
    return std::filesystem::is_regular_file(ruta_, error);
}

std::optional<std::string> ArchivoPartida::leer() const {
    std::ifstream archivo(ruta_, std::ios::binary);

    if (!archivo) {
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(archivo), std::istreambuf_iterator<char>());
}

bool ArchivoPartida::guardar(const std::string& contenido) const {
    std::error_code error;

    if (ruta_.has_parent_path()) {
        std::filesystem::create_directories(ruta_.parent_path(), error);

        if (error) {
            return false;
        }
    }
    auto temporal = ruta_;
    const auto marca = std::chrono::steady_clock::now().time_since_epoch().count();
    temporal += ".tmp-" + std::to_string(marca);
    std::ofstream archivo(temporal, std::ios::binary | std::ios::trunc);

    if (!archivo) {
        return false;
    }
    archivo << contenido;
    archivo.close();

    if (archivo) {
        std::filesystem::rename(temporal, ruta_, error);

        if (!error) {
            return true;
        }
    }
    std::filesystem::remove(temporal, error);
    return false;
}

} // namespace veterinaria
