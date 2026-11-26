// Pruebas en consola del Rol 2 (mision). Sin interfaz grafica ni dependencias.
// Usa un combate y un planificador simulados a partir de sus interfaces.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_mision
//       pruebas/pruebas_mision.cpp src/modelo/mision/*.cpp
//       src/modelo/mision/combate/combate.cpp src/modelo/mision/mapa/planificador_mapa.cpp
//   ./build/pruebas_mision

#include "modelo/mision/mision_base.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using veterinaria::Aparicion;
using veterinaria::CantidadSuministro;
using veterinaria::Combate;
using veterinaria::DesenlaceMision;
using veterinaria::EstadoCombate;
using veterinaria::MisionBase;
using veterinaria::MotivoFalloMision;
using veterinaria::OrientacionHorizontal;
using veterinaria::PlanificadorMapa;
using veterinaria::PropuestaMision;
using veterinaria::Punto;
using veterinaria::ReceptorEventosMision;
using veterinaria::Terreno;
using veterinaria::TipoEnemigo;

int comprobaciones = 0;
int fallos = 0;

void comprobar(bool condicion, const std::string& descripcion) {
    ++comprobaciones;
    if (!condicion) {
        ++fallos;
        std::cerr << "FALLA: " << descripcion << '\n';
    }
}

// Terreno fijo que entrega el planificador simulado.
const Punto inicio{500.0F, 500.0F};
const Punto vendas_a{600.0F, 500.0F};
const Punto vendas_b{700.0F, 500.0F};
const Punto antiseptico_c{500.0F, 650.0F};

Terreno terreno_de_prueba() {
    Terreno terreno;
    terreno.limites = {{0.0F, 0.0F}, 1000.0F, 1000.0F};
    terreno.inicio = inicio;
    terreno.suministros = {
        {{"vendas", 1}, vendas_a}, {{"vendas", 1}, vendas_b}, {{"antiseptico", 1}, antiseptico_c}};
    return terreno;
}

PropuestaMision caso_mora(int limite_segundos = 300) {
    PropuestaMision propuesta;
    propuesta.id_caso = "caso_mora";
    propuesta.requeridos = {{"vendas", 2}, {"antiseptico", 1}};
    propuesta.limite_segundos = limite_segundos;
    return propuesta;
}

class PlanificadorSimulado final : public PlanificadorMapa {
  public:
    veterinaria::Terreno preparar(const PropuestaMision&) override {
        ++preparaciones;
        segundos = 0.0F;
        siguiente = 0;
        return terreno_de_prueba();
    }

    std::vector<Aparicion> avanzar(float paso) override {
        segundos += paso;
        std::vector<Aparicion> vencidas;
        while (siguiente < guion.size() && guion[siguiente].first <= segundos) {
            vencidas.push_back(guion[siguiente].second);
            ++siguiente;
        }
        return vencidas;
    }

    std::vector<std::pair<float, Aparicion>> guion;
    int preparaciones = 0;

  private:
    float segundos = 0.0F;
    std::size_t siguiente = 0;
};

class CombateSimulado final : public Combate {
  public:
    void iniciar(const Terreno& terreno) override {
        ++inicios;
        estado_.jugador = terreno.inicio;
        estado_.vida = 100;
        estado_.movimiento_jugador = {OrientacionHorizontal::derecha, false};
        estado_.enemigos.clear();
        estado_.proyectiles.clear();
    }

    void agregar_enemigo(const Aparicion& aparicion) override {
        ++enemigos_agregados;
        estado_.enemigos.push_back({aparicion.tipo, aparicion.posicion, 10, 1.0F, 1, {}});
    }

    void actualizar(Punto direccion, float segundos) override {
        ultima_direccion = direccion;
        ultimo_paso = segundos;
        segundos_simulados += segundos;
    }

    const EstadoCombate& estado() const override {
        return estado_;
    }

    // Ayudas de la prueba: mueven al veterinario sin simular el combate real.
    void colocar_jugador(Punto posicion) {
        estado_.jugador = posicion;
    }
    void fijar_vida(int vida) {
        estado_.vida = vida;
    }

    int inicios = 0;
    int enemigos_agregados = 0;
    Punto ultima_direccion{0.0F, 0.0F};
    float ultimo_paso = 0.0F;
    float segundos_simulados = 0.0F;

  private:
    EstadoCombate estado_{};
};

class ReceptorGrabador final : public ReceptorEventosMision {
  public:
    void suministro_recogido(const CantidadSuministro& suministro) override {
        eventos.push_back("recogido:" + suministro.id + ":" + std::to_string(suministro.cantidad));
    }

    void intento_finalizado(DesenlaceMision desenlace) override {
        eventos.push_back(desenlace == DesenlaceMision::completada ? "fin:completada"
                                                                   : "fin:fallida");
    }

    std::vector<std::string> eventos;
};

int recogido(const MisionBase& mision, const std::string& id) {
    int total = 0;
    for (const CantidadSuministro& suministro : mision.estado().recogidos) {
        if (suministro.id == id) {
            total += suministro.cantidad;
        }
    }
    return total;
}

const Punto sin_movimiento{0.0F, 0.0F};

// ---------------------------------------------------------------------------

// Prueba 1: dos objetos del mismo tipo se acumulan y cada uno se retira una sola vez.
void prueba_acumulacion() {
    PlanificadorSimulado planificador;
    CombateSimulado combate;
    MisionBase mision(planificador, combate);
    mision.iniciar(caso_mora());

    comprobar(mision.estado().terreno.suministros.size() == 3,
              "al iniciar hay 3 objetos en el mapa");
    comprobar(mision.estado().segundos_restantes == 300.0F, "el reloj empieza en el limite");

    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 0.1F);
    comprobar(recogido(mision, "vendas") == 1, "la primera venda se recoge por contacto");
    comprobar(mision.estado().terreno.suministros.size() == 2, "el objeto recogido sale del mapa");

    mision.actualizar(sin_movimiento, 0.1F);
    comprobar(recogido(mision, "vendas") == 1, "el mismo objeto no se recoge dos veces");

    combate.colocar_jugador(vendas_b);
    mision.actualizar(sin_movimiento, 0.1F);
    comprobar(recogido(mision, "vendas") == 2, "las unidades del mismo tipo se acumulan");
    comprobar(mision.estado().recogidos.size() == 1, "las vendas quedan en una sola entrada");
    comprobar(!mision.extraer_resultado().has_value(), "con un suministro pendiente sigue activa");
}

// Prueba 2: el intento sigue hasta recoger lo pendiente o agotar el tiempo.
void prueba_pendiente_y_resultado() {
    PlanificadorSimulado planificador;
    CombateSimulado combate;
    MisionBase mision(planificador, combate);
    mision.iniciar(caso_mora());

    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 1.0F);
    combate.colocar_jugador(vendas_b);
    mision.actualizar(sin_movimiento, 1.0F);
    for (int i = 0; i < 10; ++i) {
        mision.actualizar(sin_movimiento, 1.0F);
    }
    comprobar(!mision.extraer_resultado().has_value(), "con el antiseptico pendiente sigue activa");

    combate.colocar_jugador(antiseptico_c);
    mision.actualizar(sin_movimiento, 1.0F);
    const auto resultado = mision.extraer_resultado();
    comprobar(resultado.has_value(), "al recoger lo ultimo hay resultado");
    if (resultado) {
        comprobar(resultado->desenlace == DesenlaceMision::completada,
                  "el desenlace es completada");
        comprobar(resultado->id_caso == "caso_mora", "el resultado lleva el id del caso");
        comprobar(!resultado->motivo_fallo.has_value(), "completada no lleva motivo de fallo");
        int vendas = 0;
        int antiseptico = 0;
        for (const CantidadSuministro& s : resultado->recogidos) {
            vendas += s.id == "vendas" ? s.cantidad : 0;
            antiseptico += s.id == "antiseptico" ? s.cantidad : 0;
        }
        comprobar(vendas == 2 && antiseptico == 1, "el resultado lleva las cantidades por id");
    }
    comprobar(!mision.extraer_resultado().has_value(), "la segunda extraccion devuelve vacio");

    // Tiempo agotado con un suministro pendiente.
    mision.iniciar(caso_mora(10));
    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 4.0F);
    combate.colocar_jugador(inicio);
    mision.actualizar(sin_movimiento, 5.0F);
    comprobar(!mision.extraer_resultado().has_value(), "a 1 s del limite sigue activa");
    mision.actualizar(sin_movimiento, 1.0F);
    const auto agotada = mision.extraer_resultado();
    comprobar(agotada.has_value() && agotada->desenlace == DesenlaceMision::fallida,
              "al agotarse el tiempo el intento falla");
    comprobar(agotada.has_value() && agotada->motivo_fallo == MotivoFalloMision::tiempo_agotado,
              "el motivo es tiempo_agotado");
    comprobar(agotada.has_value() && !agotada->recogidos.empty(),
              "el fallo conserva lo que se alcanzo a recoger");
}

// Prueba 3: limites del ultimo cuadro.
void prueba_ultimo_cuadro() {
    PlanificadorSimulado planificador;
    CombateSimulado combate;
    MisionBase mision(planificador, combate);

    // Vida cero: fallo, aunque este sobre un suministro.
    mision.iniciar(caso_mora());
    combate.colocar_jugador(vendas_a);
    combate.fijar_vida(0);
    mision.actualizar(sin_movimiento, 1.0F);
    const auto sin_vida = mision.extraer_resultado();
    comprobar(sin_vida.has_value() && sin_vida->motivo_fallo == MotivoFalloMision::sin_vida,
              "con vida cero se registra sin_vida");
    comprobar(recogido(mision, "vendas") == 0, "con vida cero no se procesa la recoleccion");

    // Con vida y el ultimo suministro justo en el ultimo instante: exito.
    PropuestaMision unica;
    unica.id_caso = "caso_toby";
    unica.requeridos = {{"antiseptico", 1}};
    unica.limite_segundos = 10;
    mision.iniciar(unica);
    mision.actualizar(sin_movimiento, 9.0F);
    combate.colocar_jugador(antiseptico_c);
    mision.actualizar(sin_movimiento, 1.0F);
    const auto justo = mision.extraer_resultado();
    comprobar(mision.estado().segundos_restantes == 0.0F, "el reloj llego a cero en ese cuadro");
    comprobar(justo.has_value() && justo->desenlace == DesenlaceMision::completada,
              "recoger lo ultimo en el ultimo instante cuenta como exito");

    // Vida cero en el ultimo instante tambien es fallo.
    mision.iniciar(unica);
    mision.actualizar(sin_movimiento, 9.0F);
    combate.colocar_jugador(antiseptico_c);
    combate.fijar_vida(0);
    mision.actualizar(sin_movimiento, 1.0F);
    const auto muerta = mision.extraer_resultado();
    comprobar(muerta.has_value() && muerta->motivo_fallo == MotivoFalloMision::sin_vida,
              "sin vida en el ultimo instante es fallo");
}

// Prueba 4: otro intento restaura objetos, tiempo y combate.
void prueba_reintento() {
    PlanificadorSimulado planificador;
    CombateSimulado combate;
    MisionBase mision(planificador, combate);

    mision.iniciar(caso_mora());
    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 50.0F);
    combate.fijar_vida(0);
    mision.actualizar(sin_movimiento, 1.0F);
    comprobar(mision.estado().segundos_restantes == 249.0F, "el reloj descuenta lo jugado");

    mision.iniciar(caso_mora());
    comprobar(mision.estado().terreno.suministros.size() == 3,
              "el nuevo intento recupera los objetos");
    comprobar(mision.estado().segundos_restantes == 300.0F, "el nuevo intento reinicia el reloj");
    comprobar(mision.estado().recogidos.empty(), "el nuevo intento vacia el inventario");
    comprobar(!mision.extraer_resultado().has_value(), "el resultado anterior no sobrevive");
    comprobar(combate.inicios == 2 && planificador.preparaciones == 2,
              "se prepararon de nuevo el terreno y el combate");

    // Se puede volver a jugar y completar.
    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 1.0F);
    combate.colocar_jugador(vendas_b);
    mision.actualizar(sin_movimiento, 1.0F);
    combate.colocar_jugador(antiseptico_c);
    mision.actualizar(sin_movimiento, 1.0F);
    const auto resultado = mision.extraer_resultado();
    comprobar(resultado.has_value() && resultado->desenlace == DesenlaceMision::completada,
              "el segundo intento se puede completar");
}

// Reloj, oleadas y entrada del jugador.
void prueba_reloj_y_oleadas() {
    PlanificadorSimulado planificador;
    planificador.guion = {{5.0F, {TipoEnemigo::animal_de_contacto, {10.0F, 10.0F}}},
                          {8.0F, {TipoEnemigo::superviviente_a_distancia, {20.0F, 20.0F}}}};
    CombateSimulado combate;
    MisionBase mision(planificador, combate);
    mision.iniciar(caso_mora(10));

    mision.actualizar({1.0F, 0.0F}, 4.0F);
    comprobar(combate.enemigos_agregados == 0, "antes de su segundo no pasa ninguna aparicion");
    comprobar(combate.ultima_direccion.x == 1.0F, "la direccion llega al combate");
    mision.actualizar(sin_movimiento, 1.0F);
    comprobar(combate.enemigos_agregados == 1, "la aparicion de los 5 s llega al combate una vez");
    mision.actualizar(sin_movimiento, 1.0F);
    comprobar(combate.enemigos_agregados == 1, "no se repite la aparicion");
    mision.actualizar(sin_movimiento, 2.0F);
    comprobar(combate.enemigos_agregados == 2, "la aparicion de los 8 s llega en su paso");

    // El combate nunca recibe mas tiempo del que queda.
    mision.actualizar(sin_movimiento, 5.0F);
    comprobar(combate.ultimo_paso == 2.0F, "el ultimo paso se recorta al tiempo restante");
    comprobar(combate.segundos_simulados == 10.0F, "el combate simulo exactamente el limite");

    // Terminada: el reloj y el combate se congelan.
    const float restante = mision.estado().segundos_restantes;
    mision.actualizar(sin_movimiento, 3.0F);
    comprobar(mision.estado().segundos_restantes == restante, "el reloj no avanza al terminar");
    comprobar(combate.segundos_simulados == 10.0F, "el combate no avanza al terminar");

    // Antes de iniciar no pasa nada.
    PlanificadorSimulado otro_planificador;
    CombateSimulado otro_combate;
    MisionBase sin_iniciar(otro_planificador, otro_combate);
    sin_iniciar.actualizar(sin_movimiento, 1.0F);
    comprobar(otro_combate.segundos_simulados == 0.0F, "sin iniciar no se simula nada");
    comprobar(!sin_iniciar.extraer_resultado().has_value(), "sin iniciar no hay resultado");
}

// Observer.
void prueba_receptores() {
    PlanificadorSimulado planificador;
    CombateSimulado combate;
    MisionBase mision(planificador, combate);
    ReceptorGrabador receptor;
    ReceptorGrabador baja;
    mision.suscribir(receptor);
    mision.suscribir(receptor);
    mision.suscribir(baja);
    mision.desuscribir(baja);
    mision.iniciar(caso_mora());

    combate.colocar_jugador(vendas_a);
    mision.actualizar(sin_movimiento, 1.0F);
    combate.colocar_jugador(vendas_b);
    mision.actualizar(sin_movimiento, 1.0F);
    combate.colocar_jugador(antiseptico_c);
    mision.actualizar(sin_movimiento, 1.0F);
    mision.actualizar(sin_movimiento, 1.0F);
    mision.actualizar(sin_movimiento, 1.0F);

    const std::vector<std::string> esperados{"recogido:vendas:1", "recogido:vendas:1",
                                             "recogido:antiseptico:1", "fin:completada"};
    comprobar(receptor.eventos == esperados,
              "un aviso por objeto, y el fin una sola vez y despues del ultimo suministro");
    comprobar(baja.eventos.empty(), "un receptor dado de baja no recibe avisos");

    // Dos objetos en el mismo cuadro: dos avisos, y el inventario ya esta actualizado.
    receptor.eventos.clear();
    PlanificadorSimulado planificador_junto;
    CombateSimulado combate_junto;
    MisionBase junto(planificador_junto, combate_junto, 500.0F);
    junto.suscribir(receptor);
    junto.iniciar(caso_mora());
    junto.actualizar(sin_movimiento, 1.0F);
    comprobar(receptor.eventos.size() == 4 && receptor.eventos.front() == "recogido:vendas:1" &&
                  receptor.eventos.back() == "fin:completada",
              "con varios objetos en contacto se avisa de cada uno y luego del fin");

    // Fallo: un solo aviso de fin.
    receptor.eventos.clear();
    mision.iniciar(caso_mora(5));
    mision.actualizar(sin_movimiento, 5.0F);
    mision.actualizar(sin_movimiento, 5.0F);
    comprobar(receptor.eventos == std::vector<std::string>{"fin:fallida"},
              "un fallo avisa una sola vez");
}

} // namespace

int main() {
    prueba_acumulacion();
    prueba_pendiente_y_resultado();
    prueba_ultimo_cuadro();
    prueba_reintento();
    prueba_reloj_y_oleadas();
    prueba_receptores();

    std::cout << comprobaciones - fallos << " de " << comprobaciones
              << " comprobaciones correctas.\n";
    return fallos == 0 ? 0 : 1;
}