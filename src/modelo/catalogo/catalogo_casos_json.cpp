#include "catalogo_casos_json.h"

#include "json/valor_json.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace veterinaria {

namespace {

using json::TipoJson;
using json::ValorJson;

[[noreturn]] void fallar(const std::string& campo, const std::string& motivo) {
    throw std::invalid_argument(campo + ": " + motivo);
}

std::string ruta_de(const std::string& ruta, const std::string& clave) {
    return ruta.empty() ? clave : ruta + "." + clave;
}

// Las funciones que devuelven referencias reciben const char* para no crear
// cadenas temporales que GCC marca como posibles referencias colgantes.
const ValorJson& miembro(const ValorJson& objeto, const char* ruta, const char* clave) {
    const ValorJson* valor = objeto.buscar(clave);
    const std::string campo = ruta_de(ruta, clave);

    if (valor == nullptr) {
        fallar(campo, "falta el campo");
    }

    return *valor;
}

const ValorJson& objeto_en(const ValorJson& padre, const char* ruta, const char* clave) {
    const ValorJson& valor = miembro(padre, ruta, clave);

    if (valor.tipo() != TipoJson::objeto) {
        fallar(ruta_de(ruta, clave), "se esperaba un objeto");
    }

    return valor;
}

const std::vector<ValorJson>& lista_en(const ValorJson& padre, const char* ruta,
                                       const char* clave) {
    const ValorJson& valor = miembro(padre, ruta, clave);

    if (valor.tipo() != TipoJson::lista) {
        fallar(ruta_de(ruta, clave), "se esperaba una lista");
    }

    return valor.elementos();
}

bool solo_espacios(const std::string& texto) {
    return std::all_of(texto.begin(), texto.end(),
                       [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; });
}

std::string texto_en(const ValorJson& padre, const std::string& ruta, const std::string& clave) {
    const ValorJson& valor = miembro(padre, ruta.c_str(), clave.c_str());

    if (valor.tipo() != TipoJson::cadena) {
        fallar(ruta_de(ruta, clave), "se esperaba un texto");
    }

    if (solo_espacios(valor.como_cadena())) {
        fallar(ruta_de(ruta, clave), "no puede estar vacio");
    }

    return valor.como_cadena();
}

/// Las rutas de arte se escriben desde la raiz del proyecto con separadores '/'.
std::string ruta_recurso_en(const ValorJson& padre, const std::string& ruta,
                            const std::string& clave) {
    std::string recurso = texto_en(padre, ruta, clave);

    if (recurso.find('\\') != std::string::npos) {
        fallar(ruta_de(ruta, clave), "usa '/' como separador");
    }

    if (recurso.front() == '/') {
        fallar(ruta_de(ruta, clave), "debe ser relativa a la raiz del proyecto");
    }

    return recurso;
}

int entero_en(const ValorJson& padre, const std::string& ruta, const std::string& clave,
              int minimo) {
    const ValorJson& valor = miembro(padre, ruta.c_str(), clave.c_str());
    const std::string campo = ruta_de(ruta, clave);

    if (!valor.es_entero()) {
        fallar(campo, "se esperaba un numero entero");
    }

    const double numero = valor.como_numero();

    if (numero < minimo || numero > std::numeric_limits<int>::max()) {
        fallar(campo, "debe ser un entero mayor o igual a " + std::to_string(minimo));
    }

    return static_cast<int>(numero);
}

Hablante leer_hablante(const ValorJson& linea, const std::string& ruta) {
    const std::string hablante = texto_en(linea, ruta, "hablante");

    if (hablante == "dueno") {
        return Hablante::dueno;
    }

    if (hablante == "veterinario") {
        return Hablante::veterinario;
    }

    fallar(ruta + ".hablante", "debe ser \"dueno\" o \"veterinario\"");
}

std::vector<LineaDialogo> leer_etapa(const ValorJson& dialogos, const std::string& etapa) {
    const std::string ruta = "dialogos." + etapa;
    std::vector<LineaDialogo> lineas;
    const auto& elementos = lista_en(dialogos, "dialogos", etapa.c_str());

    for (std::size_t i = 0; i < elementos.size(); ++i) {
        const std::string ruta_linea = ruta + "[" + std::to_string(i) + "]";

        if (elementos[i].tipo() != TipoJson::objeto) {
            fallar(ruta_linea, "se esperaba un objeto con hablante y texto");
        }

        lineas.push_back(
            {leer_hablante(elementos[i], ruta_linea), texto_en(elementos[i], ruta_linea, "texto")});
    }

    return lineas;
}

std::vector<CantidadSuministro> leer_suministros(const ValorJson& raiz) {
    const auto& elementos = lista_en(raiz, "", "suministros");

    if (elementos.empty()) {
        fallar("suministros", "se requiere al menos un suministro");
    }

    std::vector<CantidadSuministro> suministros;
    std::set<std::string> identificadores;

    for (std::size_t i = 0; i < elementos.size(); ++i) {
        const std::string ruta = "suministros[" + std::to_string(i) + "]";

        if (elementos[i].tipo() != TipoJson::objeto) {
            fallar(ruta, "se esperaba un objeto con id y cantidad");
        }

        const std::string id = texto_en(elementos[i], ruta, "id");

        if (!identificadores.insert(id).second) {
            fallar(ruta + ".id", "suministro repetido: " + id);
        }

        suministros.push_back({id, entero_en(elementos[i], ruta, "cantidad", 1)});
    }

    return suministros;
}

std::string leer_archivo(const std::filesystem::path& archivo) {
    std::ifstream entrada(archivo, std::ios::binary);

    if (!entrada) {
        throw std::invalid_argument("no se pudo abrir el archivo");
    }

    std::ostringstream contenido;
    contenido << entrada.rdbuf();
    return contenido.str();
}

} // namespace

CatalogoCasosJson::CatalogoCasosJson(const FabricaMascotas& fabrica) : fabrica_(fabrica) {
}

CatalogoCasosJson::~CatalogoCasosJson() = default;

CasoClinico CatalogoCasosJson::interpretar(const std::string& texto) const {
    ValorJson raiz;

    try {
        raiz = json::leer_json(texto);
    }

    catch (const json::ErrorJson& error) {
        throw std::invalid_argument(std::string("JSON mal formado: ") + error.what());
    }

    if (raiz.tipo() != TipoJson::objeto) {
        fallar("ficha", "se esperaba un objeto JSON");
    }

    CasoClinico caso;
    caso.id = texto_en(raiz, "", "id");

    const ValorJson& mascota = objeto_en(raiz, "", "mascota");
    caso.mascota.id = texto_en(mascota, "mascota", "id");
    caso.mascota.nombre = texto_en(mascota, "mascota", "nombre");
    caso.mascota.especie = texto_en(mascota, "mascota", "especie");
    caso.mascota.edad_meses = entero_en(mascota, "mascota", "edad_meses", 0);
    caso.mascota.retrato = ruta_recurso_en(mascota, "mascota", "retrato");

    if (!fabrica_.crear(caso.mascota)) {
        fallar("mascota.especie", "especie no admitida: " + caso.mascota.especie);
    }

    const ValorJson& dueno = objeto_en(raiz, "", "dueno");
    caso.dueno.id = texto_en(dueno, "dueno", "id");
    caso.dueno.nombre = texto_en(dueno, "dueno", "nombre");
    caso.dueno.sprite = ruta_recurso_en(dueno, "dueno", "sprite");

    const ValorJson& dialogos = objeto_en(raiz, "", "dialogos");
    caso.dialogos.llegada = leer_etapa(dialogos, "llegada");
    caso.dialogos.aceptacion = leer_etapa(dialogos, "aceptacion");
    caso.dialogos.rechazo = leer_etapa(dialogos, "rechazo");
    caso.dialogos.fallo = leer_etapa(dialogos, "fallo");
    caso.dialogos.tratamiento = leer_etapa(dialogos, "tratamiento");

    caso.suministros = leer_suministros(raiz);
    caso.recompensa = entero_en(raiz, "", "recompensa", 0);

    return caso;
}

CargaCasos CatalogoCasosJson::cargar(const std::filesystem::path& directorio) const {
    CargaCasos carga;
    std::error_code error;

    if (!std::filesystem::is_directory(directorio, error)) {
        carga.errores.push_back({directorio, "no es un directorio de fichas"});
        return carga;
    }

    std::vector<std::filesystem::path> archivos;
    for (const auto& entrada : std::filesystem::directory_iterator(directorio, error)) {
        if (entrada.is_regular_file() && entrada.path().extension() == ".json") {
            archivos.push_back(entrada.path());
        }
    }

    if (error) {
        carga.errores.push_back({directorio, "no se pudo recorrer: " + error.message()});
    }

    // El nombre de archivo fija el orden de llegada de los casos.
    std::sort(archivos.begin(), archivos.end(),
              [](const auto& a, const auto& b) { return a.filename() < b.filename(); });

    std::set<std::string> identificadores;

    for (const auto& archivo : archivos) {
        try {
            CasoClinico caso = interpretar(leer_archivo(archivo));

            if (!identificadores.insert(caso.id).second) {
                fallar("id", "identificador de caso repetido: " + caso.id);
            }

            carga.casos.push_back(std::move(caso));
        }

        catch (const std::invalid_argument& motivo) {
            // Una ficha mal formada no impide cargar las demas.
            carga.errores.push_back({archivo, motivo.what()});
        }
    }

    return carga;
}

} // namespace veterinaria
