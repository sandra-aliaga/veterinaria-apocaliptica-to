// Pruebas en consola del Rol 1 (cola y consultorio). Sin interfaz grafica ni dependencias.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_consultorio
//       pruebas/pruebas_consultorio.cpp src/modelo/clinica/*.cpp
//   ./build/pruebas_consultorio

#include "modelo/clinica/cola_casos.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using veterinaria::CantidadSuministro;
using veterinaria::CasoClinico;
using veterinaria::ColaCasosClinicos;

int g_comprobaciones = 0;
int g_fallos = 0;

void comprobar(bool condicion, const char* texto, const char* archivo, int linea) {
    ++g_comprobaciones;
    if (!condicion) {
        ++g_fallos;
        std::cerr << archivo << ':' << linea << ": FALLO: " << texto << '\n';
    }
}

#define COMPROBAR(condicion) comprobar((condicion), #condicion, __FILE__, __LINE__)

/// Construye una ficha manualmente, sin catalogo ni JSON.
CasoClinico crear_caso(const std::string& id, int recompensa,
                       std::vector<CantidadSuministro> suministros) {
    CasoClinico caso;
    caso.id = id;
    caso.mascota = {"m-" + id, "Mascota " + id, "perro", 12, ""};
    caso.dueno = {"d-" + id, "Dueno " + id, ""};
    caso.suministros = std::move(suministros);
    caso.recompensa = recompensa;
    return caso;
}

CasoClinico caso_a() {
    return crear_caso("caso-a", 100, {{"vendas", 2}, {"antibiotico", 1}});
}

CasoClinico caso_b() {
    return crear_caso("caso-b", 50, {{"suero", 3}});
}

void prueba_cola_vacia() {
    ColaCasosClinicos cola;
    COMPROBAR(cola.frente() == nullptr);
    COMPROBAR(!cola.extraer().has_value());
    COMPROBAR(cola.frente() == nullptr);
}

void prueba_cola_orden_fifo() {
    ColaCasosClinicos cola;
    cola.encolar(caso_a());
    cola.encolar(caso_b());

    COMPROBAR(cola.frente() != nullptr && cola.frente()->id == "caso-a");
    COMPROBAR(cola.frente() != nullptr && cola.frente()->id == "caso-a"); // frente no retira
    auto primero = cola.extraer();
    COMPROBAR(primero.has_value() && primero->id == "caso-a");
    COMPROBAR(primero.has_value() && primero->suministros.size() == 2);
    COMPROBAR(cola.frente() != nullptr && cola.frente()->id == "caso-b");
    auto segundo = cola.extraer();
    COMPROBAR(segundo.has_value() && segundo->id == "caso-b");
    COMPROBAR(!cola.extraer().has_value());
}

void prueba_cola_reinsercion_tras_vaciar() {
    ColaCasosClinicos cola;
    cola.encolar(caso_a());
    COMPROBAR(cola.extraer().has_value()); // un unico elemento: ultimo_ debe quedar nulo
    COMPROBAR(cola.frente() == nullptr);

    cola.encolar(caso_b());
    cola.encolar(caso_a());
    COMPROBAR(cola.frente() != nullptr && cola.frente()->id == "caso-b");
    auto primero = cola.extraer();
    auto segundo = cola.extraer();
    COMPROBAR(primero.has_value() && primero->id == "caso-b");
    COMPROBAR(segundo.has_value() && segundo->id == "caso-a");
    COMPROBAR(!cola.extraer().has_value());
}

void prueba_cola_larga_se_libera() {
    ColaCasosClinicos cola;
    for (int i = 0; i < 200000; ++i) {
        cola.encolar(crear_caso("c" + std::to_string(i), 1, {}));
    }
    // La destruccion no debe desbordar la pila.
}

} // namespace

int main() {
    prueba_cola_vacia();
    prueba_cola_orden_fifo();
    prueba_cola_reinsercion_tras_vaciar();
    prueba_cola_larga_se_libera();

    std::cout << g_comprobaciones << " comprobaciones, " << g_fallos << " fallos.\n";
    return g_fallos == 0 ? 0 : 1;
}
