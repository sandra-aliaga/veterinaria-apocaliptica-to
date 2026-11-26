// Pruebas en consola del Rol 3 (catalogo y mascotas). Sin interfaz grafica ni dependencias.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_catalogo
//       pruebas/pruebas_catalogo.cpp src/modelo/catalogo/*.cpp
//       src/modelo/catalogo/json/*.cpp src/modelo/catalogo/mascota/*.cpp
//   ./build/pruebas_catalogo

#include "modelo/catalogo/catalogo_casos_json.h"
#include "modelo/catalogo/json/valor_json.h"
#include "modelo/catalogo/mascota/fabrica_mascotas_base.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

namespace fs = std::filesystem;

using veterinaria::CargaCasos;
using veterinaria::CatalogoCasosJson;
using veterinaria::FabricaMascotasBase;
using veterinaria::FichaMascota;
using veterinaria::Hablante;
using veterinaria::Mascota;
using veterinaria::json::ErrorJson;
using veterinaria::json::leer_json;
using veterinaria::json::TipoJson;

int comprobaciones = 0;
int fallos = 0;

void comprobar(bool condicion, const std::string& descripcion) {
    ++comprobaciones;
    if (!condicion) {
        ++fallos;
        std::cerr << "FALLA: " << descripcion << '\n';
    }
}

bool cerca(double a, double b) {
    return std::fabs(a - b) < 1e-9;
}

bool contiene(const std::string& texto, const std::string& parte) {
    return texto.find(parte) != std::string::npos;
}

void escribir(const fs::path& archivo, const std::string& contenido) {
    std::ofstream salida(archivo, std::ios::binary);
    salida << contenido;
}

/// Lee el archivo sin '\r' para que los reemplazos funcionen con CRLF o LF.
std::string leer(const fs::path& archivo) {
    std::ifstream entrada(archivo, std::ios::binary);
    std::string texto{std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>()};
    texto.erase(std::remove(texto.begin(), texto.end(), '\r'), texto.end());
    return texto;
}

/// Carpeta temporal que se borra al terminar cada prueba.
class CarpetaTemporal {
  public:
    explicit CarpetaTemporal(const std::string& nombre)
        : ruta_(fs::temp_directory_path() / nombre) {
        fs::remove_all(ruta_);
        fs::create_directories(ruta_);
    }

    ~CarpetaTemporal() {
        std::error_code ignorado;
        fs::remove_all(ruta_, ignorado);
    }

    CarpetaTemporal(const CarpetaTemporal&) = delete;
    CarpetaTemporal& operator=(const CarpetaTemporal&) = delete;

    const fs::path& ruta() const {
        return ruta_;
    }

  private:
    fs::path ruta_;
};

void probar_mascotas() {
    const FabricaMascotasBase fabrica;

    const std::unique_ptr<Mascota> gato = fabrica.crear({"mora", "Mora", "gato", 24, "m.png"});
    const std::unique_ptr<Mascota> perro = fabrica.crear({"toby", "Toby", "perro", 48, "t.png"});
    const std::unique_ptr<Mascota> cachorro = fabrica.crear({"c", "C", "perro", 6, "c.png"});

    comprobar(gato != nullptr && perro != nullptr, "La fabrica crea gatos y perros");
    comprobar(fabrica.crear({"x", "X", "loro", 12, "x.png"}) == nullptr,
              "La fabrica devuelve nulo para especies no admitidas");

    comprobar(gato && cerca(gato->calcularEdadHumana(), 24.0), "Un gato de 2 anios equivale a 24");
    comprobar(perro && cerca(perro->calcularEdadHumana(), 34.0),
              "Un perro de 4 anios equivale a 34");
    comprobar(cachorro && cerca(cachorro->calcularEdadHumana(), 7.5),
              "Un cachorro de 6 meses equivale a 7,5");

    const std::unique_ptr<Mascota> gato_mayor = fabrica.crear({"g", "G", "gato", 48, "g.png"});
    comprobar(gato_mayor && cerca(gato_mayor->calcularEdadHumana(), 32.0),
              "Gato y perro envejecen distinto desde el tercer anio");
    comprobar(gato && gato->ficha().retrato == "m.png", "La mascota conserva su ficha");
}

void probar_lector_json() {
    const auto valor = leer_json("{\"a\": [1, 2.5, true, null], \"b\": \"\\u00f1and\\u00fa\"}");
    comprobar(valor.tipo() == TipoJson::objeto, "Lee un objeto");
    comprobar(valor.claves().size() == 2 && valor.claves()[0] == "a",
              "Conserva el orden de las claves");

    const auto* lista = valor.buscar("a");
    comprobar(lista && lista->elementos().size() == 4, "Lee una lista");
    comprobar(lista && lista->elementos()[0].es_entero(), "Distingue enteros");
    comprobar(lista && !lista->elementos()[1].es_entero(), "Distingue decimales");
    comprobar(valor.buscar("b") && valor.buscar("b")->como_cadena() == "\xC3\xB1"
                                                                       "and\xC3\xBA",
              "Convierte escapes \\u a UTF-8");

    bool lanzo = false;
    try {
        leer_json("{\n  \"a\": 1,\n  \"b\": }");
    }

    catch (const ErrorJson& error) {
        lanzo = error.linea() == 3;
    }
    comprobar(lanzo, "Informa la linea de un error de sintaxis");

    lanzo = false;
    try {
        leer_json("{\"a\": 1, \"a\": 2}");
    }

    catch (const ErrorJson&) {
        lanzo = true;
    }
    comprobar(lanzo, "Rechaza claves repetidas");
}

void probar_fichas_del_repositorio() {
    const FabricaMascotasBase fabrica;
    const CatalogoCasosJson catalogo(fabrica);
    const CargaCasos carga = catalogo.cargar("datos");

    comprobar(carga.errores.empty(), "Las fichas del repositorio no tienen errores");
    comprobar(carga.casos.size() == 2, "Se cargan las dos fichas de la demo");

    if (carga.casos.size() != 2) {
        return;
    }

    const auto& mora = carga.casos[0];
    const auto& toby = carga.casos[1];
    comprobar(mora.id == "caso_mora" && toby.id == "caso_toby", "Se respeta el orden por nombre");
    comprobar(mora.mascota.retrato == "arte/mascotas/mora.png" &&
                  toby.mascota.retrato == "arte/mascotas/toby.png",
              "Se conservan los retratos");
    comprobar(mora.dueno.sprite == "arte/duenos/elena.png", "Se conserva el sprite del dueno");
    comprobar(fs::exists(mora.mascota.retrato) && fs::exists(toby.mascota.retrato) &&
                  fs::exists(mora.dueno.sprite) && fs::exists(toby.dueno.sprite),
              "Las rutas de arte existen en el repositorio");

    comprobar(mora.suministros.size() == 2 && mora.suministros[0].id == "vendas" &&
                  mora.suministros[0].cantidad == 2 && mora.suministros[1].cantidad == 1,
              "Mora pide dos vendas y un antiseptico");
    comprobar(toby.suministros.size() == 1 && toby.suministros[0].id == "suero",
              "Toby pide un solo tipo de suministro");

    comprobar(mora.dialogos.llegada.size() == 4, "Una etapa conserva varias lineas");
    comprobar(mora.dialogos.llegada[1].hablante == Hablante::veterinario,
              "Se conserva quien habla en cada linea");
    comprobar(!mora.dialogos.tratamiento.empty() && !toby.dialogos.fallo.empty(),
              "Se conservan todas las etapas de dialogo");
    comprobar(contiene(mora.dialogos.aceptacion[0].texto, "antiséptico"),
              "El texto UTF-8 se conserva");

    const auto& mirada = toby.dialogos.llegada.at(3);
    const auto& tratamiento = toby.dialogos.tratamiento.front();
    comprobar(mirada.sprite_dueno == "arte/duenos/dueno_toby_mueca.png" &&
                  fs::exists(mirada.sprite_dueno),
              "La frase de Toby conserva la reaccion del dueno");
    comprobar(tratamiento.hablante == Hablante::veterinario &&
                  tratamiento.retrato_mascota == "arte/mascotas/toby_recuperado.png" &&
                  fs::exists(tratamiento.retrato_mascota),
              "La mascota puede reaccionar mientras habla el veterinario");
    comprobar(toby.dialogos.llegada.front().sprite_dueno.empty() &&
                  toby.dialogos.llegada.front().retrato_mascota.empty(),
              "Una linea sin reacciones conserva los retratos base");
}

void probar_ficha_invalida() {
    const CarpetaTemporal carpeta("veterinaria_pruebas_catalogo");
    fs::copy_file("datos/01_mora.json", carpeta.ruta() / "01_mora.json");
    fs::copy_file("datos/02_toby.json", carpeta.ruta() / "02_toby.json");
    escribir(carpeta.ruta() / "notas.txt", "No es una ficha.");

    std::string tercera = leer("datos/02_toby.json");
    tercera.replace(tercera.find("caso_toby"), 9, "caso_rex");
    tercera.replace(tercera.find("\"cantidad\": 1"), 13, "\"cantidad\": 0");
    escribir(carpeta.ruta() / "03_rex.json", tercera);

    const FabricaMascotasBase fabrica;
    const CatalogoCasosJson catalogo(fabrica);
    const CargaCasos carga = catalogo.cargar(carpeta.ruta());

    comprobar(carga.casos.size() == 2, "Las otras dos fichas siguen disponibles");
    comprobar(carga.errores.size() == 1, "Se informa un solo error");

    if (carga.errores.size() == 1) {
        comprobar(carga.errores[0].archivo.filename() == "03_rex.json",
                  "El error indica el archivo");
        comprobar(contiene(carga.errores[0].detalle, "suministros[0].cantidad"),
                  "El error indica el campo: " + carga.errores[0].detalle);
    }
}

void probar_cambio_de_cantidad() {
    const CarpetaTemporal carpeta("veterinaria_pruebas_cantidad");
    std::string mora = leer("datos/01_mora.json");
    mora.replace(mora.find("\"cantidad\": 2"), 13, "\"cantidad\": 5");
    escribir(carpeta.ruta() / "01_mora.json", mora);

    const FabricaMascotasBase fabrica;
    const CargaCasos carga = CatalogoCasosJson(fabrica).cargar(carpeta.ruta());
    comprobar(carga.casos.size() == 1 && carga.casos[0].suministros[0].cantidad == 5,
              "Al cambiar la cantidad el catalogo entrega el nuevo valor");
}

void probar_validaciones() {
    const FabricaMascotasBase fabrica;
    const CatalogoCasosJson catalogo(fabrica);
    const std::string base = leer("datos/02_toby.json");

    const auto motivo = [&](const std::string& buscado, const std::string& reemplazo) {
        std::string ficha = base;
        ficha.replace(ficha.find(buscado), buscado.size(), reemplazo);
        try {
            catalogo.interpretar(ficha);
        }

        catch (const std::invalid_argument& error) {
            return std::string(error.what());
        }
        return std::string();
    };

    comprobar(contiene(motivo("\"perro\"", "\"loro\""), "mascota.especie"),
              "Se rechaza una especie no admitida");
    comprobar(contiene(motivo("\"edad_meses\": 48", "\"edad_meses\": -1"), "mascota.edad_meses"),
              "Se rechaza una edad negativa");
    comprobar(contiene(motivo("\"recompensa\": 25", "\"recompensa\": 2.5"), "recompensa"),
              "Se rechaza una recompensa decimal");
    comprobar(contiene(motivo("\"hablante\": \"dueno\"", "\"hablante\": \"gato\""),
                       "dialogos.llegada[0].hablante"),
              "Se rechaza un hablante desconocido");
    comprobar(contiene(motivo("arte/duenos/dueno_toby_mueca.png", "/reaccion.png"),
                       "dialogos.llegada[3].sprite_dueno"),
              "Una ruta de reaccion invalida identifica la linea del dialogo");
    comprobar(contiene(motivo("\"suministros\": [", "\"suministros\": [{\"id\": \"suero\", "
                                                    "\"cantidad\": 1}, "),
                       "suministro repetido"),
              "Se rechaza un suministro repetido");
    comprobar(contiene(motivo("arte/mascotas/toby.png", "arte\\\\mascotas\\\\toby.png"),
                       "mascota.retrato"),
              "Se exige el separador '/' en las rutas");
    comprobar(contiene(motivo("\"rechazo\": [", "\"rechazo_x\": ["), "dialogos.rechazo"),
              "Se exige cada etapa de dialogo");
    comprobar(contiene(motivo("}\n  ],\n  \"recompensa\"", "}\n  ,\n  \"recompensa\""),
                       "JSON mal formado"),
              "Se informa un JSON mal formado");
}

void probar_identificador_repetido() {
    const CarpetaTemporal carpeta("veterinaria_pruebas_repetido");
    fs::copy_file("datos/01_mora.json", carpeta.ruta() / "01_mora.json");
    fs::copy_file("datos/01_mora.json", carpeta.ruta() / "02_copia.json");

    const FabricaMascotasBase fabrica;
    const CargaCasos carga = CatalogoCasosJson(fabrica).cargar(carpeta.ruta());
    comprobar(carga.casos.size() == 1 && carga.errores.size() == 1 &&
                  carga.errores[0].archivo.filename() == "02_copia.json",
              "Un identificador de caso repetido se informa en el segundo archivo");

    const CargaCasos sin_directorio = CatalogoCasosJson(fabrica).cargar(carpeta.ruta() / "nada");
    comprobar(sin_directorio.casos.empty() && sin_directorio.errores.size() == 1,
              "Un directorio inexistente se informa como error");
}

} // namespace

int main() {
    probar_mascotas();
    probar_lector_json();
    probar_fichas_del_repositorio();
    probar_ficha_invalida();
    probar_cambio_de_cantidad();
    probar_validaciones();
    probar_identificador_repetido();

    std::cout << comprobaciones - fallos << '/' << comprobaciones << " comprobaciones correctas\n";
    return fallos == 0 ? 0 : 1;
}
