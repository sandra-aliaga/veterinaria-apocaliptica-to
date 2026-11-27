// Pruebas en consola del Rol 4 (combate). Sin interfaz grafica ni dependencias.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_combate
//       pruebas/pruebas_combate.cpp src/modelo/mision/combate/*.cpp
//       src/modelo/mision/combate/arma/*.cpp src/modelo/mision/combate/enemigo/*.cpp
//   ./build/pruebas_combate

#include "controlador/configuracion_juego.h"
#include "modelo/mision/combate/arma/arma_base.h"
#include "modelo/mision/combate/combate_base.h"
#include "modelo/mision/combate/enemigo/atacar_a_distancia.h"
#include "modelo/mision/combate/enemigo/enemigo_con_estrategia.h"
#include "modelo/mision/combate/enemigo/fabrica_enemigos_base.h"
#include "modelo/mision/combate/enemigo/perseguir_por_contacto.h"
#include "modelo/mision/combate/veterinario_jugable.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

namespace {

using veterinaria::Aparicion;
using veterinaria::ArmaBase;
using veterinaria::AtacarADistancia;
using veterinaria::AtaqueEnemigo;
using veterinaria::CombateBase;
using veterinaria::ConfiguracionCombate;
using veterinaria::Enemigo;
using veterinaria::EnemigoConEstrategia;
using veterinaria::EstadoEnemigo;
using veterinaria::EstrategiaEnemigo;
using veterinaria::FabricaEnemigos;
using veterinaria::FabricaEnemigosBase;
using veterinaria::IntencionEnemigo;
using veterinaria::ObstaculoMapa;
using veterinaria::OrientacionHorizontal;
using veterinaria::PerseguirPorContacto;
using veterinaria::Punto;
using veterinaria::Rectangulo;
using veterinaria::Terreno;
using veterinaria::TipoEnemigo;
using veterinaria::TipoObstaculo;
using veterinaria::VeterinarioJugable;

int comprobaciones = 0;
int fallos = 0;

void comprobar(bool condicion, const std::string& descripcion) {
    ++comprobaciones;
    if (!condicion) {
        ++fallos;
        std::cerr << "FALLA: " << descripcion << '\n';
    }
}

constexpr int danio_arma = 10;
constexpr float intervalo_arma = 0.5F;
constexpr float alcance_arma = 96.0F;
constexpr float cuadro = 1.0F / 60.0F;

/// Terreno fijo de 400x300 con una roca a la derecha del inicio.
Terreno terreno_fijo() {
    Terreno terreno;
    terreno.limites = {{0.0F, 0.0F}, 400.0F, 300.0F};
    terreno.inicio = {100.0F, 150.0F};
    terreno.obstaculos.push_back(
        {TipoObstaculo::roca, {160.0F, 150.0F}, {{150.0F, 130.0F}, 20.0F, 40.0F}});
    return terreno;
}

/// Estrategia que deja al enemigo quieto y sin atacar.
class Quieto final : public EstrategiaEnemigo {
  public:
    IntencionEnemigo decidir(const EstadoEnemigo& /*enemigo*/, Punto /*jugador*/,
                             float /*segundos*/) override {
        return {{0.0F, 0.0F}, AtaqueEnemigo::ninguno};
    }
};

/// Fabrica de prueba: todos los enemigos se quedan quietos con 100 de vida.
class FabricaQuieta final : public FabricaEnemigos {
  public:
    std::unique_ptr<Enemigo> crear(const Aparicion& aparicion) const override {
        const EstadoEnemigo estado{aparicion.tipo,
                                   aparicion.posicion,
                                   100,
                                   0.0F,
                                   0,
                                   {OrientacionHorizontal::derecha, false}};
        return std::make_unique<EnemigoConEstrategia>(estado, std::make_unique<Quieto>());
    }
};

CombateBase crear_combate(std::unique_ptr<FabricaEnemigos> fabrica) {
    return CombateBase(std::move(fabrica),
                       std::make_unique<ArmaBase>(danio_arma, intervalo_arma, alcance_arma));
}

void simular(CombateBase& combate, Punto direccion, float segundos) {
    for (float t = 0.0F; t < segundos; t += cuadro) {
        combate.actualizar(direccion, cuadro);
    }
}

void probar_veterinario() {
    VeterinarioJugable veterinario({10.0F, 10.0F}, 100, 120.0F);
    comprobar(veterinario.movimiento().orientacion == OrientacionHorizontal::derecha,
              "El veterinario empieza mirando a la derecha");

    veterinario.mover_a({5.0F, 10.0F});
    comprobar(veterinario.movimiento().orientacion == OrientacionHorizontal::izquierda,
              "Moverse a la izquierda cambia la orientacion");
    comprobar(veterinario.movimiento().en_movimiento, "Moverse marca en_movimiento");

    veterinario.mover_a({5.0F, 10.0F});
    comprobar(veterinario.movimiento().orientacion == OrientacionHorizontal::izquierda,
              "Detenerse conserva la ultima orientacion");
    comprobar(!veterinario.movimiento().en_movimiento, "Detenerse apaga en_movimiento");

    veterinario.recibir_danio(2);
    veterinario.curar(3);
    comprobar(veterinario.vida() == 100, "La curacion no supera la vida maxima");
    veterinario.recibir_danio(250);
    veterinario.curar(1);
    comprobar(veterinario.vida() == 0, "La vida no baja de cero");
}

void probar_arma() {
    ArmaBase arma(danio_arma, intervalo_arma, alcance_arma);
    comprobar(arma.alcance() == alcance_arma, "El arma conserva su alcance configurado");
    comprobar(arma.intentar_atacar() == danio_arma, "El arma empieza lista");
    comprobar(arma.intentar_atacar() == 0, "Tras atacar el arma recarga");

    arma.actualizar(intervalo_arma);
    comprobar(arma.intentar_atacar() == danio_arma, "Al terminar el intervalo vuelve a golpear");

    arma.reiniciar();
    comprobar(arma.intentar_atacar() == danio_arma, "Reiniciar deja el arma lista");
}

void probar_limites_y_obstaculo() {
    auto combate = crear_combate(std::make_unique<FabricaQuieta>());
    combate.iniciar(terreno_fijo());
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {350.0F, 50.0F}});
    combate.agregar_enemigo({TipoEnemigo::superviviente_de_contacto, {350.0F, 250.0F}});
    comprobar(combate.estado().enemigos.size() == 2, "Se agregan dos apariciones manuales");

    simular(combate, {1.0F, 0.0F}, 2.0F);
    const auto& tras_roca = combate.estado();
    comprobar(tras_roca.jugador.x + 8.0F <= 150.0F + 0.01F, "La roca bloquea el paso");

    simular(combate, {-1.0F, 0.0F}, 3.0F);
    comprobar(std::fabs(combate.estado().jugador.x - 8.0F) < 0.01F,
              "El veterinario no sale del limite izquierdo");

    combate.actualizar({0.0F, 0.0F}, cuadro);
    comprobar(combate.estado().movimiento_jugador.orientacion == OrientacionHorizontal::izquierda,
              "Al detenerse mantiene la ultima orientacion");
    comprobar(!combate.estado().movimiento_jugador.en_movimiento,
              "Al detenerse deja de estar en movimiento");
}

void probar_contacto_y_recarga() {
    auto combate = crear_combate(std::make_unique<FabricaEnemigosBase>());
    combate.iniciar(terreno_fijo());
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {100.0F, 60.0F}});

    simular(combate, {0.0F, 0.0F}, 1.5F);
    const auto& estado = combate.estado();
    comprobar(estado.vida < estado.vida_maxima, "El enemigo de contacto hiere al veterinario");
    comprobar(estado.instante_danio && *estado.instante_danio <= estado.segundos_combate,
              "El danio publica el instante del impacto");
    comprobar(estado.enemigos.empty() ||
                  estado.enemigos.front().vida <
                      FabricaEnemigosBase::perfil(TipoEnemigo::animal_de_contacto).vida,
              "El arma responde al contacto");

    simular(combate, {0.0F, 0.0F}, 3.0F);
    comprobar(combate.estado().enemigos.empty(), "El enemigo eliminado se retira");
}

void probar_latigo_automatico() {
    auto combate = crear_combate(std::make_unique<FabricaQuieta>());
    combate.iniciar(terreno_fijo());
    combate.actualizar({0.0F, 0.0F}, cuadro);

    const auto barrido_sin_objetivo = combate.estado().ultimo_golpe;
    comprobar(barrido_sin_objetivo && !barrido_sin_objetivo->impacto,
              "El latigo se anima incluso sin enemigos");
    comprobar(barrido_sin_objetivo && barrido_sin_objetivo->alcance == alcance_arma,
              "La vista recibe el mismo alcance que usa el modelo");

    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {135.0F, 145.0F}});
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {190.0F, 171.0F}});
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {80.0F, 150.0F}});
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {220.0F, 150.0F}});
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {147.0F, 80.0F}});
    combate.actualizar({0.0F, 0.0F}, cuadro);
    const auto en_recarga = combate.estado().ultimo_golpe;
    comprobar(en_recarga && barrido_sin_objetivo &&
                  en_recarga->gesto.instante == barrido_sin_objetivo->gesto.instante &&
                  combate.estado().enemigos.at(0).vida == 100,
              "Un ataque en vacio consume la recarga");

    simular(combate, {0.0F, 0.0F}, intervalo_arma);
    const auto& enemigos = combate.estado().enemigos;
    comprobar(enemigos.at(0).vida == 100 - danio_arma && enemigos.at(1).vida == 100 - danio_arma,
              "Un barrido golpea a todos los enemigos dentro de su area");
    comprobar(enemigos.at(2).vida == 100 && enemigos.at(3).vida == 100 &&
                  enemigos.at(4).vida == 100,
              "Deja intactos a los enemigos detras, lejos o fuera del ancho del barrido");
    comprobar(!enemigos.at(0).variante_visual && enemigos.at(1).variante_visual,
              "Los animales alternan sus variantes visuales al aparecer");

    const auto golpe = combate.estado().ultimo_golpe;
    comprobar(golpe && golpe->impacto && golpe->impacto->x == 135.0F,
              "El golpe con objetivo publica una posicion de impacto");

    combate.actualizar({-1.0F, 0.0F}, cuadro);
    simular(combate, {0.0F, 0.0F}, intervalo_arma);
    const auto vuelto = combate.estado().ultimo_golpe;
    comprobar(vuelto && vuelto->gesto.orientacion == OrientacionHorizontal::izquierda &&
                  combate.estado().enemigos.at(2).vida == 100 - danio_arma,
              "Al voltear el latigo alcanza el otro lado");
}

void probar_cura_e_invulnerabilidad() {
    CombateBase combate(std::make_unique<FabricaEnemigosBase>(),
                        std::make_unique<ArmaBase>(32, intervalo_arma, alcance_arma));
    combate.iniciar(terreno_fijo());

    for (int i = 0; i < 3; ++i) {
        combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {84.0F, 150.0F}});
    }

    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {130.0F, 145.0F}});
    combate.agregar_enemigo({TipoEnemigo::animal_de_contacto, {135.0F, 160.0F}});
    combate.actualizar({0.0F, 0.0F}, cuadro);
    comprobar(combate.estado().enemigos.size() == 3, "El latigo logra dos bajas al frente");
    comprobar(combate.estado().vida == 92,
              "El grupo inflige un golpe y dos bajas curan un punto cada una");
    combate.actualizar({0.0F, 0.0F}, cuadro);
    comprobar(combate.estado().vida == 92, "La proteccion breve evita que se acumule el danio");
}

void probar_ataque_a_distancia() {
    AtacarADistancia estrategia(220.0F, 2.0F);
    const EstadoEnemigo cerca{
        TipoEnemigo::superviviente_a_distancia, {100.0F, 100.0F}, 40, 50.0F, 8,
        {OrientacionHorizontal::derecha, false}};
    AtacarADistancia guardia(220.0F, 2.0F, true);
    const auto desde_su_puesto = guardia.decidir(cerca, {150.0F, 100.0F}, 2.0F);
    comprobar(desde_su_puesto.direccion.x == 0.0F && desde_su_puesto.direccion.y == 0.0F &&
                  desde_su_puesto.ataque == AtaqueEnemigo::proyectil,
              "El guardia dispara sin abandonar su puesto");

    const auto huida = estrategia.decidir(cerca, {150.0F, 100.0F}, 0.1F);
    comprobar(huida.direccion.x < 0.0F, "Retrocede si el veterinario esta demasiado cerca");

    auto combate = crear_combate(std::make_unique<FabricaEnemigosBase>());
    combate.iniciar(terreno_fijo());
    combate.agregar_enemigo({TipoEnemigo::superviviente_a_distancia, {100.0F, 290.0F - 8.0F}});

    bool hubo_proyectil = false;
    bool hubo_direccion = false;
    bool hubo_abanico = false;
    for (float t = 0.0F; t < 4.0F; t += cuadro) {
        combate.actualizar({0.0F, 0.0F}, cuadro);
        const auto& proyectiles = combate.estado().proyectiles;
        hubo_proyectil = hubo_proyectil || !proyectiles.empty();
        hubo_direccion =
            hubo_direccion || (!proyectiles.empty() && proyectiles.front().velocidad.y < 0);

        if (proyectiles.size() == 5 && !hubo_abanico) {
            const auto& primero = proyectiles.front();
            const auto& ultimo = proyectiles.back();
            hubo_abanico = primero.origen.x == ultimo.origen.x &&
                           primero.origen.y == ultimo.origen.y &&
                           primero.posicion.y < primero.origen.y &&
                           primero.velocidad.x * ultimo.velocidad.x < 0.0F;
        }
    }

    comprobar(hubo_proyectil, "El superviviente a distancia genera proyectiles");
    comprobar(hubo_direccion, "El estado publica la direccion de vuelo");
    comprobar(hubo_abanico, "Cinco rayos abren un abanico desde un mismo origen");
    comprobar(combate.estado().vida < combate.estado().vida_maxima,
              "Los proyectiles hieren al veterinario");

    const auto& enemigos = combate.estado().enemigos;
    comprobar(!enemigos.empty() && enemigos.front().disparo.has_value(),
              "El tirador publica su gesto al disparar");
    comprobar(!enemigos.empty() &&
                  std::hypot(enemigos.front().posicion.x - combate.estado().jugador.x,
                             enemigos.front().posicion.y - combate.estado().jugador.y) > 50.0F,
              "El superviviente mantiene distancia");

    combate.iniciar(terreno_fijo());
    const auto& reiniciado = combate.estado();
    comprobar(reiniciado.enemigos.empty() && reiniciado.proyectiles.empty(),
              "Iniciar limpia enemigos y proyectiles");
    comprobar(reiniciado.vida == reiniciado.vida_maxima, "Iniciar restaura la vida");
    comprobar(reiniciado.segundos_combate == 0.0F && !reiniciado.instante_danio &&
                  !reiniciado.ultimo_golpe,
              "Un combate nuevo reinicia el reloj y sus efectos");
    comprobar(reiniciado.jugador.x == 100.0F && reiniciado.jugador.y == 150.0F,
              "Iniciar coloca al veterinario en el inicio");
}

void probar_dos_latigazos_al_vigia() {
    namespace ajustes = veterinaria::configuracion_juego;
    CombateBase combate(std::make_unique<FabricaEnemigosBase>(),
                        std::make_unique<ArmaBase>(ajustes::danio_arma, ajustes::recarga_arma,
                                                   ajustes::alcance_arma));
    Terreno terreno = terreno_fijo();
    terreno.obstaculos.clear();

    for (const bool guardia : {false, true}) {
        combate.iniciar(terreno);
        const Punto puesto{terreno.inicio.x + 110.0F, terreno.inicio.y};
        combate.agregar_enemigo({TipoEnemigo::superviviente_a_distancia, puesto, guardia});
        combate.actualizar({0.0F, 0.0F}, cuadro);
        const auto& tras_golpe = combate.estado().enemigos;
        comprobar(tras_golpe.size() == 1 && tras_golpe.front().vida == 8 &&
                      tras_golpe.front().vida_maxima == 40,
                  "El vigia sobrevive al primer latigazo con ocho puntos de vida");
        simular(combate, {0.0F, 0.0F}, ajustes::recarga_arma);
        comprobar(combate.estado().enemigos.empty(), "El segundo latigazo elimina al vigia");
    }
}

void probar_danio_de_estela() {
    CombateBase combate(std::make_unique<FabricaEnemigosBase>(), nullptr);
    Terreno terreno = terreno_fijo();
    terreno.obstaculos.clear();
    combate.iniciar(terreno);
    combate.agregar_enemigo({TipoEnemigo::superviviente_a_distancia, {100.0F, 280.0F}, true});

    simular(combate, {0.0F, 0.0F}, 2.03F);
    simular(combate, {1.0F, 0.0F}, 0.15F);
    simular(combate, {0.0F, 0.0F}, 0.82F);
    comprobar(combate.estado().proyectiles.size() == 5 &&
                  combate.estado().vida == combate.estado().vida_maxima,
              "El veterinario puede esperar en el espacio entre rayos");
    comprobar(combate.estado().proyectiles.at(2).posicion.y < terreno.inicio.y - 20.0F,
              "La punta del rayo ya paso al veterinario antes de cruzar");

    simular(combate, {-1.0F, 0.0F}, 0.12F);
    const auto& rayos_restantes = combate.estado().proyectiles;
    const auto es_rayo_central = [](const auto& rayo) { return std::abs(rayo.velocidad.x) < 1.0F; };
    comprobar(combate.estado().vida == combate.estado().vida_maxima - 8 &&
                  std::none_of(rayos_restantes.begin(), rayos_restantes.end(), es_rayo_central),
              "Cruzar la estela inflige danio y consume el rayo despues de pasar su punta");
}

void probar_duracion_rafaga() {
    ConfiguracionCombate configuracion;
    configuracion.duracion_proyectil = 0.20F;
    CombateBase combate(std::make_unique<FabricaEnemigosBase>(), nullptr, configuracion);
    combate.iniciar(terreno_fijo());
    combate.agregar_enemigo({TipoEnemigo::superviviente_a_distancia, {100.0F, 280.0F}, true});
    simular(combate, {0.0F, 0.0F}, 2.05F);
    comprobar(combate.estado().proyectiles.size() == 5, "Se ven los cinco rayos durante el vuelo");
    simular(combate, {0.0F, 0.0F}, 0.20F);
    comprobar(combate.estado().proyectiles.empty(),
              "Los rayos y sus origenes dejan de publicarse al vencer su duracion");
}

void probar_sin_vida() {
    ConfiguracionCombate configuracion;
    configuracion.vida_jugador = 10;
    CombateBase combate(std::make_unique<FabricaEnemigosBase>(),
                        std::make_unique<ArmaBase>(danio_arma, intervalo_arma, alcance_arma),
                        configuracion);
    combate.iniciar(terreno_fijo());
    combate.agregar_enemigo({TipoEnemigo::superviviente_de_contacto, {100.0F, 130.0F}});

    simular(combate, {0.0F, 0.0F}, 1.0F);
    const Punto final = combate.estado().jugador;
    comprobar(combate.estado().vida == 0, "El veterinario puede quedarse sin vida");

    simular(combate, {1.0F, 0.0F}, 0.5F);
    comprobar(combate.estado().jugador.x == final.x, "Sin vida el veterinario ya no se mueve");
}

} // namespace

int main() {
    probar_veterinario();
    probar_arma();
    probar_limites_y_obstaculo();
    probar_contacto_y_recarga();
    probar_latigo_automatico();
    probar_cura_e_invulnerabilidad();
    probar_ataque_a_distancia();
    probar_dos_latigazos_al_vigia();
    probar_danio_de_estela();
    probar_duracion_rafaga();
    probar_sin_vida();

    std::cout << comprobaciones - fallos << '/' << comprobaciones << " comprobaciones correctas\n";
    return fallos == 0 ? 0 : 1;
}
