// Pruebas en consola del Rol 1 (cola y consultorio). Sin interfaz grafica ni dependencias.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_consultorio
//       pruebas/pruebas_consultorio.cpp src/modelo/clinica/*.cpp
//   ./build/pruebas_consultorio

#include "modelo/clinica/cola_casos.h"
#include "modelo/clinica/consultorio_clinica.h"

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using veterinaria::CantidadSuministro;
using veterinaria::CasoClinico;
using veterinaria::ColaCasosClinicos;
using veterinaria::ConsultorioClinica;
using veterinaria::DesenlaceMision;
using veterinaria::MotivoFalloMision;
using veterinaria::PropuestaMision;
using veterinaria::RegistroMision;
using veterinaria::ResultadoMision;

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

ResultadoMision exito(const std::string& id, std::vector<CantidadSuministro> recogidos) {
    return {id, DesenlaceMision::completada, std::move(recogidos), std::nullopt};
}

ResultadoMision fallo(const std::string& id, MotivoFalloMision motivo) {
    return {id, DesenlaceMision::fallida, {}, motivo};
}

bool mismos_requisitos(const std::vector<CantidadSuministro>& a,
                       const std::vector<CantidadSuministro>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].id != b[i].id || a[i].cantidad != b[i].cantidad) {
            return false;
        }
    }
    return true;
}

void prueba_consultorio_orden_y_reinsercion() {
    ConsultorioClinica consultorio;
    COMPROBAR(consultorio.ficha_pendiente() == nullptr);
    COMPROBAR(consultorio.caso_activo() == nullptr);
    COMPROBAR(consultorio.saldo() == 0);
    COMPROBAR(!consultorio.aceptar().has_value()); // cola vacia: sin efecto
    COMPROBAR(!consultorio.rechazar());

    consultorio.recibir(caso_a());
    consultorio.recibir(caso_b());
    COMPROBAR(consultorio.ficha_pendiente() != nullptr &&
              consultorio.ficha_pendiente()->id == "caso-a");

    COMPROBAR(consultorio.rechazar());
    COMPROBAR(consultorio.rechazar());
    COMPROBAR(consultorio.ficha_pendiente() == nullptr);

    consultorio.recibir(caso_b()); // reinsercion tras vaciar
    COMPROBAR(consultorio.ficha_pendiente() != nullptr &&
              consultorio.ficha_pendiente()->id == "caso-b");
}

void prueba_aceptar_entrega_propuesta() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    consultorio.recibir(caso_b());

    const auto propuesta = consultorio.aceptar();
    COMPROBAR(propuesta.has_value());
    COMPROBAR(propuesta.has_value() && propuesta->id_caso == "caso-a");
    COMPROBAR(propuesta.has_value() && propuesta->limite_segundos == 300);
    COMPROBAR(propuesta.has_value() &&
              mismos_requisitos(propuesta->requeridos, caso_a().suministros));
    COMPROBAR(consultorio.caso_activo() != nullptr && consultorio.caso_activo()->id == "caso-a");
    COMPROBAR(consultorio.ficha_pendiente() != nullptr &&
              consultorio.ficha_pendiente()->id == "caso-b");

    // Solo hay un intento activo: aceptar y rechazar no tienen efecto.
    COMPROBAR(!consultorio.aceptar().has_value());
    COMPROBAR(!consultorio.rechazar());
    COMPROBAR(consultorio.caso_activo() != nullptr && consultorio.caso_activo()->id == "caso-a");
    COMPROBAR(consultorio.ficha_pendiente() != nullptr &&
              consultorio.ficha_pendiente()->id == "caso-b");
    COMPROBAR(!consultorio.reintentar().has_value()); // el intento aun esta en curso
}

void prueba_sin_caso_activo() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    COMPROBAR(
        consultorio.registrar_resultado(exito("caso-a", {{"vendas", 2}, {"antibiotico", 1}})) ==
        RegistroMision::sin_caso_activo);
    COMPROBAR(consultorio.saldo() == 0);
    COMPROBAR(consultorio.ficha_pendiente() != nullptr); // la ficha sigue en la cola
}

void prueba_exito_incompleto_no_cierra_ni_cobra() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    consultorio.aceptar();

    // Falta un suministro.
    COMPROBAR(consultorio.registrar_resultado(exito("caso-a", {{"vendas", 2}})) ==
              RegistroMision::resultado_invalido);
    // Cantidad insuficiente.
    COMPROBAR(
        consultorio.registrar_resultado(exito("caso-a", {{"vendas", 1}, {"antibiotico", 1}})) ==
        RegistroMision::resultado_invalido);
    // Lista vacia.
    COMPROBAR(consultorio.registrar_resultado(exito("caso-a", {})) ==
              RegistroMision::resultado_invalido);
    // El ID no coincide.
    COMPROBAR(
        consultorio.registrar_resultado(exito("caso-b", {{"vendas", 2}, {"antibiotico", 1}})) ==
        RegistroMision::resultado_invalido);

    COMPROBAR(consultorio.saldo() == 0);
    COMPROBAR(consultorio.caso_activo() != nullptr && consultorio.caso_activo()->id == "caso-a");

    // El intento sigue vivo: un resultado completo todavia puede cerrarlo.
    COMPROBAR(consultorio.registrar_resultado(
                  exito("caso-a", {{"vendas", 2}, {"antibiotico", 1}})) == RegistroMision::tratado);
    COMPROBAR(consultorio.saldo() == 100);
}

void prueba_fallo_y_reintento() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    consultorio.recibir(caso_b());
    const auto original = consultorio.aceptar();
    COMPROBAR(original.has_value());

    // Un fallo sin motivo no es un resultado valido.
    COMPROBAR(
        consultorio.registrar_resultado({"caso-a", DesenlaceMision::fallida, {}, std::nullopt}) ==
        RegistroMision::resultado_invalido);

    COMPROBAR(consultorio.registrar_resultado(fallo("caso-a", MotivoFalloMision::sin_vida)) ==
              RegistroMision::fallo_registrado);
    COMPROBAR(consultorio.caso_activo() != nullptr && consultorio.caso_activo()->id == "caso-a");
    COMPROBAR(consultorio.caso_activo() != nullptr && consultorio.caso_activo()->recompensa == 100);
    COMPROBAR(consultorio.saldo() == 0);

    // El intento ya se registro: repetir el fallo es invalido y no se puede aceptar otro caso.
    COMPROBAR(consultorio.registrar_resultado(fallo("caso-a", MotivoFalloMision::sin_vida)) ==
              RegistroMision::resultado_invalido);
    COMPROBAR(!consultorio.aceptar().has_value());

    const auto reintento = consultorio.reintentar();
    COMPROBAR(reintento.has_value());
    COMPROBAR(reintento.has_value() && original.has_value() &&
              reintento->id_caso == original->id_caso);
    COMPROBAR(reintento.has_value() && original.has_value() &&
              mismos_requisitos(reintento->requeridos, original->requeridos));
    COMPROBAR(reintento.has_value() && reintento->limite_segundos == 300);
    COMPROBAR(!consultorio.reintentar().has_value()); // un solo intento activo

    // Un segundo fallo (por tiempo) tambien permite reintentar.
    COMPROBAR(consultorio.registrar_resultado(fallo("caso-a", MotivoFalloMision::tiempo_agotado)) ==
              RegistroMision::fallo_registrado);
    COMPROBAR(consultorio.reintentar().has_value());
    COMPROBAR(consultorio.registrar_resultado(
                  exito("caso-a", {{"antibiotico", 1}, {"vendas", 3}})) == RegistroMision::tratado);
    COMPROBAR(consultorio.saldo() == 100);
    COMPROBAR(consultorio.caso_activo() == nullptr);
    COMPROBAR(!consultorio.reintentar().has_value()); // el caso ya se cerro

    // La cola continua con la siguiente ficha.
    const auto siguiente = consultorio.aceptar();
    COMPROBAR(siguiente.has_value() && siguiente->id_caso == "caso-b");
}

void prueba_cobro_unico() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    consultorio.recibir(caso_b());
    consultorio.aceptar();

    const ResultadoMision completo = exito("caso-a", {{"vendas", 2}, {"antibiotico", 1}});
    COMPROBAR(consultorio.registrar_resultado(completo) == RegistroMision::tratado);
    COMPROBAR(consultorio.saldo() == 100);

    // Repetir el mismo resultado no vuelve a cobrar.
    COMPROBAR(consultorio.registrar_resultado(completo) == RegistroMision::sin_caso_activo);
    COMPROBAR(consultorio.saldo() == 100);

    // Tampoco cobra si ya hay otro caso activo con otro ID.
    COMPROBAR(consultorio.aceptar().has_value());
    COMPROBAR(consultorio.registrar_resultado(completo) == RegistroMision::resultado_invalido);
    COMPROBAR(consultorio.saldo() == 100);
    COMPROBAR(consultorio.registrar_resultado(exito("caso-b", {{"suero", 3}})) ==
              RegistroMision::tratado);
    COMPROBAR(consultorio.saldo() == 150);
}

void prueba_rechazar_avanza_la_cola() {
    ConsultorioClinica consultorio;
    consultorio.recibir(caso_a());
    consultorio.recibir(caso_b());

    COMPROBAR(consultorio.rechazar());
    COMPROBAR(consultorio.caso_activo() == nullptr); // rechazar no inicia mision
    COMPROBAR(consultorio.ficha_pendiente() != nullptr &&
              consultorio.ficha_pendiente()->id == "caso-b");
    COMPROBAR(consultorio.saldo() == 0);

    const auto propuesta = consultorio.aceptar();
    COMPROBAR(propuesta.has_value() && propuesta->id_caso == "caso-b");
    COMPROBAR(consultorio.ficha_pendiente() == nullptr);
}

} // namespace

int main() {
    prueba_cola_vacia();
    prueba_cola_orden_fifo();
    prueba_cola_reinsercion_tras_vaciar();
    prueba_cola_larga_se_libera();
    prueba_consultorio_orden_y_reinsercion();
    prueba_aceptar_entrega_propuesta();
    prueba_sin_caso_activo();
    prueba_exito_incompleto_no_cierra_ni_cobra();
    prueba_fallo_y_reintento();
    prueba_cobro_unico();
    prueba_rechazar_avanza_la_cola();

    std::cout << g_comprobaciones << " comprobaciones, " << g_fallos << " fallos.\n";
    return g_fallos == 0 ? 0 : 1;
}
