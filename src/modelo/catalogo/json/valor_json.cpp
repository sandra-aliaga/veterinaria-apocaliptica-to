#include "valor_json.h"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace veterinaria::json {

// ---------------------------------------------------------------------------
// ValorJson
// ---------------------------------------------------------------------------

ValorJson::ValorJson() : tipo_(TipoJson::nulo), booleano_(false), numero_(0.0), es_entero_(false) {
}

ValorJson ValorJson::booleano(bool valor) {
    ValorJson resultado;
    resultado.tipo_ = TipoJson::booleano;
    resultado.booleano_ = valor;
    return resultado;
}

ValorJson ValorJson::numero(double valor, bool es_entero) {
    ValorJson resultado;
    resultado.tipo_ = TipoJson::numero;
    resultado.numero_ = valor;
    resultado.es_entero_ = es_entero;
    return resultado;
}

ValorJson ValorJson::cadena(std::string valor) {
    ValorJson resultado;
    resultado.tipo_ = TipoJson::cadena;
    resultado.cadena_ = std::move(valor);
    return resultado;
}

ValorJson ValorJson::lista(std::vector<ValorJson> elementos) {
    ValorJson resultado;
    resultado.tipo_ = TipoJson::lista;
    resultado.elementos_ = std::move(elementos);
    return resultado;
}

ValorJson ValorJson::objeto(std::vector<std::string> claves, std::vector<ValorJson> valores) {
    ValorJson resultado;
    resultado.tipo_ = TipoJson::objeto;
    resultado.claves_ = std::move(claves);
    resultado.elementos_ = std::move(valores);
    return resultado;
}

TipoJson ValorJson::tipo() const {
    return tipo_;
}

bool ValorJson::es_entero() const {
    return tipo_ == TipoJson::numero && es_entero_;
}

bool ValorJson::como_booleano() const {
    return booleano_;
}

double ValorJson::como_numero() const {
    return numero_;
}

const std::string& ValorJson::como_cadena() const {
    return cadena_;
}

const std::vector<ValorJson>& ValorJson::elementos() const {
    return elementos_;
}

const std::vector<std::string>& ValorJson::claves() const {
    return claves_;
}

const ValorJson* ValorJson::buscar(const std::string& clave) const {
    if (tipo_ != TipoJson::objeto) {
        return nullptr;
    }

    for (std::size_t i = 0; i < claves_.size(); ++i) {
        if (claves_[i] == clave) {
            return &elementos_[i];
        }
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// ErrorJson
// ---------------------------------------------------------------------------

ErrorJson::ErrorJson(const std::string& mensaje, std::size_t linea, std::size_t columna)
    : std::runtime_error(mensaje + " (linea " + std::to_string(linea) + ", columna " +
                         std::to_string(columna) + ")"),
      linea_(linea), columna_(columna) {
}

std::size_t ErrorJson::linea() const {
    return linea_;
}

std::size_t ErrorJson::columna() const {
    return columna_;
}

// ---------------------------------------------------------------------------
// Lector descendente recursivo
// ---------------------------------------------------------------------------

namespace {

constexpr int profundidad_maxima = 64;

class Lector {
  public:
    explicit Lector(const std::string& texto) : texto_(texto) {
        // Se admite la marca BOM que agregan algunos editores al guardar en UTF-8.
        if (texto_.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            posicion_ = 3;
        }
    }

    ValorJson documento() {
        ValorJson valor = leer_valor(0);
        saltar_espacios();

        if (posicion_ < texto_.size()) {
            fallar("Hay contenido despues del valor principal");
        }

        return valor;
    }

  private:
    [[noreturn]] void fallar(const std::string& mensaje) const {
        std::size_t linea = 1;
        std::size_t columna = 1;

        for (std::size_t i = 0; i < posicion_ && i < texto_.size(); ++i) {
            if (texto_[i] == '\n') {
                ++linea;
                columna = 1;
            }

            else {
                ++columna;
            }
        }

        throw ErrorJson(mensaje, linea, columna);
    }

    bool fin() const {
        return posicion_ >= texto_.size();
    }

    char actual() const {
        return fin() ? '\0' : texto_[posicion_];
    }

    void saltar_espacios() {
        while (!fin() &&
               (actual() == ' ' || actual() == '\t' || actual() == '\n' || actual() == '\r')) {
            ++posicion_;
        }
    }

    void esperar(char caracter) {
        saltar_espacios();

        if (actual() != caracter) {
            fallar(std::string("Se esperaba '") + caracter + "'");
        }

        ++posicion_;
    }

    ValorJson leer_valor(int profundidad) {
        if (profundidad > profundidad_maxima) {
            fallar("El documento tiene demasiados niveles");
        }

        saltar_espacios();

        switch (actual()) {
        case '{':
            return leer_objeto(profundidad);
        case '[':
            return leer_lista(profundidad);
        case '"':
            return ValorJson::cadena(leer_cadena());
        case 't':
            leer_literal("true");
            return ValorJson::booleano(true);
        case 'f':
            leer_literal("false");
            return ValorJson::booleano(false);
        case 'n':
            leer_literal("null");
            return ValorJson();
        default:
            break;
        }

        if (actual() == '-' || (actual() >= '0' && actual() <= '9')) {
            return leer_numero();
        }

        fallar(fin() ? "El documento termina antes de tiempo" : "Valor no reconocido");
    }

    void leer_literal(const char* literal) {
        const std::string esperado(literal);

        if (texto_.compare(posicion_, esperado.size(), esperado) != 0) {
            fallar("Valor no reconocido");
        }

        posicion_ += esperado.size();
    }

    ValorJson leer_objeto(int profundidad) {
        ++posicion_; // '{'
        std::vector<std::string> claves;
        std::vector<ValorJson> valores;

        saltar_espacios();
        if (actual() == '}') {
            ++posicion_;
            return ValorJson::objeto(std::move(claves), std::move(valores));
        }

        while (true) {
            saltar_espacios();
            if (actual() != '"') {
                fallar("Se esperaba una clave entre comillas");
            }

            std::string clave = leer_cadena();
            for (const auto& existente : claves) {
                if (existente == clave) {
                    fallar("Clave repetida: " + clave);
                }
            }

            esperar(':');
            claves.push_back(std::move(clave));
            valores.push_back(leer_valor(profundidad + 1));

            saltar_espacios();
            if (actual() == ',') {
                ++posicion_;
                continue;
            }

            esperar('}');
            return ValorJson::objeto(std::move(claves), std::move(valores));
        }
    }

    ValorJson leer_lista(int profundidad) {
        ++posicion_; // '['
        std::vector<ValorJson> elementos;

        saltar_espacios();
        if (actual() == ']') {
            ++posicion_;
            return ValorJson::lista(std::move(elementos));
        }

        while (true) {
            elementos.push_back(leer_valor(profundidad + 1));

            saltar_espacios();
            if (actual() == ',') {
                ++posicion_;
                continue;
            }

            esperar(']');
            return ValorJson::lista(std::move(elementos));
        }
    }

    unsigned leer_hexadecimal() {
        unsigned codigo = 0;

        for (int i = 0; i < 4; ++i) {
            const char c = actual();
            codigo <<= 4U;

            if (c >= '0' && c <= '9') {
                codigo |= static_cast<unsigned>(c - '0');
            }

            else if (c >= 'a' && c <= 'f') {
                codigo |= static_cast<unsigned>(c - 'a' + 10);
            }

            else if (c >= 'A' && c <= 'F') {
                codigo |= static_cast<unsigned>(c - 'A' + 10);
            }

            else {
                fallar("Escape \\u incompleto");
            }

            ++posicion_;
        }

        return codigo;
    }

    static void agregar_utf8(std::string& destino, unsigned codigo) {
        if (codigo < 0x80U) {
            destino += static_cast<char>(codigo);
        }

        else if (codigo < 0x800U) {
            destino += static_cast<char>(0xC0U | (codigo >> 6U));
            destino += static_cast<char>(0x80U | (codigo & 0x3FU));
        }

        else if (codigo < 0x10000U) {
            destino += static_cast<char>(0xE0U | (codigo >> 12U));
            destino += static_cast<char>(0x80U | ((codigo >> 6U) & 0x3FU));
            destino += static_cast<char>(0x80U | (codigo & 0x3FU));
        }

        else {
            destino += static_cast<char>(0xF0U | (codigo >> 18U));
            destino += static_cast<char>(0x80U | ((codigo >> 12U) & 0x3FU));
            destino += static_cast<char>(0x80U | ((codigo >> 6U) & 0x3FU));
            destino += static_cast<char>(0x80U | (codigo & 0x3FU));
        }
    }

    unsigned leer_escape_unicode() {
        unsigned codigo = leer_hexadecimal();

        // Los caracteres fuera del plano basico llegan como par sustituto.
        if (codigo >= 0xD800U && codigo <= 0xDBFFU) {
            if (texto_.compare(posicion_, 2, "\\u") != 0) {
                fallar("Falta la segunda mitad del par sustituto");
            }

            posicion_ += 2;
            const unsigned bajo = leer_hexadecimal();

            if (bajo < 0xDC00U || bajo > 0xDFFFU) {
                fallar("Par sustituto invalido");
            }

            codigo = 0x10000U + ((codigo - 0xD800U) << 10U) + (bajo - 0xDC00U);
        }

        else if (codigo >= 0xDC00U && codigo <= 0xDFFFU) {
            fallar("Par sustituto invalido");
        }

        return codigo;
    }

    std::string leer_cadena() {
        ++posicion_; // '"'
        std::string resultado;

        while (true) {
            if (fin()) {
                fallar("Falta cerrar la cadena");
            }

            const char c = actual();
            ++posicion_;

            if (c == '"') {
                return resultado;
            }

            if (static_cast<unsigned char>(c) < 0x20U) {
                fallar("Caracter de control dentro de la cadena");
            }

            if (c != '\\') {
                resultado += c;
                continue;
            }

            const char escape = actual();
            ++posicion_;

            switch (escape) {
            case '"':
            case '\\':
            case '/':
                resultado += escape;
                break;
            case 'b':
                resultado += '\b';
                break;
            case 'f':
                resultado += '\f';
                break;
            case 'n':
                resultado += '\n';
                break;
            case 'r':
                resultado += '\r';
                break;
            case 't':
                resultado += '\t';
                break;
            case 'u':
                agregar_utf8(resultado, leer_escape_unicode());
                break;
            default:
                --posicion_;
                fallar("Escape no valido");
            }
        }
    }

    ValorJson leer_numero() {
        const std::size_t inicio = posicion_;
        bool es_entero = true;

        if (actual() == '-') {
            ++posicion_;
        }

        if (actual() == '0') {
            ++posicion_;
        }

        else if (actual() >= '1' && actual() <= '9') {
            leer_digitos();
        }

        else {
            fallar("Numero mal formado");
        }

        if (actual() == '.') {
            es_entero = false;
            ++posicion_;
            if (!(actual() >= '0' && actual() <= '9')) {
                fallar("Faltan digitos despues del punto");
            }
            leer_digitos();
        }

        if (actual() == 'e' || actual() == 'E') {
            es_entero = false;
            ++posicion_;
            if (actual() == '+' || actual() == '-') {
                ++posicion_;
            }

            if (!(actual() >= '0' && actual() <= '9')) {
                fallar("Exponente mal formado");
            }
            leer_digitos();
        }

        const std::string literal = texto_.substr(inicio, posicion_ - inicio);
        errno = 0;
        const double valor = std::strtod(literal.c_str(), nullptr);

        if (errno == ERANGE) {
            fallar("Numero fuera de rango");
        }

        return ValorJson::numero(valor, es_entero);
    }

    void leer_digitos() {
        while (actual() >= '0' && actual() <= '9') {
            ++posicion_;
        }
    }

    const std::string& texto_;
    std::size_t posicion_ = 0;
};

} // namespace

ValorJson leer_json(const std::string& texto) {
    return Lector(texto).documento();
}

} // namespace veterinaria::json
