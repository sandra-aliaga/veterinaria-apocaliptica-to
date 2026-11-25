#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace veterinaria::json {

enum class TipoJson { nulo, booleano, numero, cadena, lista, objeto };

/// Valor JSON leido de una ficha. Los objetos conservan el orden de sus claves.
class ValorJson {
  public:
    ValorJson();

    static ValorJson booleano(bool valor);
    static ValorJson numero(double valor, bool es_entero);
    static ValorJson cadena(std::string valor);
    static ValorJson lista(std::vector<ValorJson> elementos);
    static ValorJson objeto(std::vector<std::string> claves, std::vector<ValorJson> valores);

    TipoJson tipo() const;
    bool es_entero() const;

    bool como_booleano() const;
    double como_numero() const;
    const std::string& como_cadena() const;
    const std::vector<ValorJson>& elementos() const;
    const std::vector<std::string>& claves() const;

    /// Devuelve el miembro con esa clave, o nulo si no es un objeto o no existe.
    const ValorJson* buscar(const std::string& clave) const;

  private:
    TipoJson tipo_;
    bool booleano_;
    double numero_;
    bool es_entero_;
    std::string cadena_;
    std::vector<ValorJson> elementos_;
    std::vector<std::string> claves_;
};

/// Error de sintaxis con la linea y la columna donde se detecto.
class ErrorJson : public std::runtime_error {
  public:
    ErrorJson(const std::string& mensaje, std::size_t linea, std::size_t columna);

    std::size_t linea() const;
    std::size_t columna() const;

  private:
    std::size_t linea_;
    std::size_t columna_;
};

/// Interpreta un documento JSON en UTF-8. Lanza ErrorJson si no es valido.
ValorJson leer_json(const std::string& texto);

} // namespace veterinaria::json
