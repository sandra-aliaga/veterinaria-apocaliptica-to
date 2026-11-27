// Pruebas en consola del Rol 2 (mapa y oleadas). Sin interfaz grafica ni dependencias.
//
// Compilar y ejecutar desde la raiz del repositorio:
//   mkdir -p build
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_mapa
//       pruebas/pruebas_mapa.cpp src/modelo/mision/mapa/*.cpp
//   ./build/pruebas_mapa

#include "modelo/mision/mapa/planificador_campo_abierto.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using veterinaria::Aparicion;
using veterinaria::CantidadSuministro;
using veterinaria::PiezaSuelo;
using veterinaria::PlanificadorCampoAbierto;
using veterinaria::PropuestaMision;
using veterinaria::Punto;
using veterinaria::Rectangulo;
using veterinaria::Terreno;
using veterinaria::TipoEnemigo;
using veterinaria::TipoPiezaSuelo;

int comprobaciones = 0;
int fallos = 0;

void comprobar(bool condicion, const std::string& descripcion) {
    ++comprobaciones;
    if (!condicion) {
        ++fallos;
        std::cerr << "FALLA: " << descripcion << '\n';
    }
}

const Rectangulo limites{{0.0F, 0.0F}, 1600.0F, 1200.0F};

Punto limites_centro() {
    return {limites.origen.x + limites.ancho / 2.0F, limites.origen.y + limites.alto / 2.0F};
}

PropuestaMision caso_mora() {
    PropuestaMision propuesta;
    propuesta.id_caso = "caso_mora";
    propuesta.requeridos = {{"vendas", 2}, {"antiseptico", 1}};
    return propuesta;
}

bool dentro(const Rectangulo& rectangulo, Punto punto) {
    return punto.x > rectangulo.origen.x && punto.x < rectangulo.origen.x + rectangulo.ancho &&
           punto.y > rectangulo.origen.y && punto.y < rectangulo.origen.y + rectangulo.alto;
}

bool se_intersectan(const Rectangulo& a, const Rectangulo& b) {
    return a.origen.x < b.origen.x + b.ancho && b.origen.x < a.origen.x + a.ancho &&
           a.origen.y < b.origen.y + b.alto && b.origen.y < a.origen.y + a.alto;
}

float distancia(Punto a, Punto b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

float distancia_a_rectangulo(Punto punto, const Rectangulo& r) {
    const float dx = std::max({r.origen.x - punto.x, 0.0F, punto.x - (r.origen.x + r.ancho)});
    const float dy = std::max({r.origen.y - punto.y, 0.0F, punto.y - (r.origen.y + r.alto)});
    return std::hypot(dx, dy);
}

bool bloqueado(const Terreno& terreno, Punto punto) {
    return std::any_of(terreno.obstaculos.begin(), terreno.obstaculos.end(),
                       [&](const auto& obstaculo) { return dentro(obstaculo.colision, punto); });
}

int total_de(const Terreno& terreno, const std::string& id) {
    int total = 0;
    for (const auto& suministro : terreno.suministros) {
        if (suministro.contenido.id == id) {
            total += suministro.contenido.cantidad;
        }
    }
    return total;
}

// Resumen de un terreno para compararlo con otro.
std::vector<float> firma(const Terreno& terreno) {
    std::vector<float> valores{terreno.inicio.x, terreno.inicio.y};
    for (const auto& obstaculo : terreno.obstaculos) {
        valores.push_back(static_cast<float>(obstaculo.tipo));
        valores.push_back(obstaculo.colision.origen.x);
        valores.push_back(obstaculo.colision.origen.y);
    }

    for (const auto& suministro : terreno.suministros) {
        valores.push_back(suministro.posicion.x);
        valores.push_back(suministro.posicion.y);
    }

    for (const PiezaSuelo& pieza : terreno.suelo.piezas) {
        valores.push_back(static_cast<float>(pieza.tipo) * 10.0F +
                          static_cast<float>(pieza.giro_cuartos));
    }
    return valores;
}

// Comprobacion independiente de la del planificador: otra resolucion y sin holgura.
bool es_alcanzable(const Terreno& terreno, Punto destino) {
    constexpr float celda = 8.0F;
    const int columnas = static_cast<int>(terreno.limites.ancho / celda);
    const int filas = static_cast<int>(terreno.limites.alto / celda);
    const auto indice = [&](int c, int f) { return static_cast<std::size_t>(f * columnas + c); };
    const auto celda_de = [&](Punto p) {
        return std::make_pair(static_cast<int>(p.x / celda), static_cast<int>(p.y / celda));
    };
    const auto libre = [&](int c, int f) {
        return !bloqueado(terreno, Punto{(static_cast<float>(c) + 0.5F) * celda,
                                         (static_cast<float>(f) + 0.5F) * celda});
    };

    std::vector<char> visitada(static_cast<std::size_t>(columnas) * filas, 0);
    std::vector<std::pair<int, int>> cola{celda_de(terreno.inicio)};
    visitada[indice(cola[0].first, cola[0].second)] = 1;
    const auto meta = celda_de(destino);

    for (std::size_t cabeza = 0; cabeza < cola.size(); ++cabeza) {
        const auto [c, f] = cola[cabeza];
        if (c == meta.first && f == meta.second) {
            return true;
        }
        const int pasos[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const auto& paso : pasos) {
            const int nc = c + paso[0];
            const int nf = f + paso[1];
            if (nc < 0 || nc >= columnas || nf < 0 || nf >= filas ||
                visitada[indice(nc, nf)] != 0 || !libre(nc, nf)) {
                continue;
            }
            visitada[indice(nc, nf)] = 1;
            cola.emplace_back(nc, nf);
        }
    }
    return false;
}

// Misma camara que la vista: lienzo de 640 x 360 centrado en el veterinario sin salir del campo.
Rectangulo camara_de(const Rectangulo& campo, Punto jugador) {
    const float x_maxima = campo.origen.x + std::max(0.0F, campo.ancho - 640.0F);
    const float y_maxima = campo.origen.y + std::max(0.0F, campo.alto - 360.0F);
    return {{std::clamp(jugador.x - 320.0F, campo.origen.x, x_maxima),
             std::clamp(jugador.y - 180.0F, campo.origen.y, y_maxima)},
            640.0F,
            360.0F};
}

// 0 arriba, 1 derecha, 2 abajo, 3 izquierda, -1 si cae dentro de la camara.
int lado_de(const Rectangulo& camara, Punto punto) {
    if (punto.y < camara.origen.y) {
        return 0;
    }

    if (punto.x > camara.origen.x + camara.ancho) {
        return 1;
    }

    if (punto.y > camara.origen.y + camara.alto) {
        return 2;
    }
    return punto.x < camara.origen.x ? 3 : -1;
}

struct Entrega {
    float segundo; // Final del paso en que se entrego.
    Aparicion aparicion;
};

std::vector<Entrega> recorrer(PlanificadorCampoAbierto& planificador, const Terreno& terreno,
                              float paso, float total, Punto jugador) {
    std::vector<Entrega> entregas;
    const int pasos = static_cast<int>(total / paso);
    for (int i = 1; i <= pasos; ++i) {
        for (const Aparicion& aparicion : planificador.avanzar(paso, jugador, terreno)) {
            entregas.push_back({static_cast<float>(i) * paso, aparicion});
        }
    }
    return entregas;
}

bool mismas_apariciones(const std::vector<Aparicion>& a, const std::vector<Entrega>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].tipo != b[i].aparicion.tipo || a[i].posicion.x != b[i].aparicion.posicion.x ||
            a[i].posicion.y != b[i].aparicion.posicion.y) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------

void prueba_suministros() {
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    comprobar(total_de(terreno, "vendas") == 2, "el mapa ofrece 2 vendas");
    comprobar(total_de(terreno, "antiseptico") == 1, "el mapa ofrece 1 antiseptico");
    comprobar(terreno.suministros.size() == 3, "hay una entrada por unidad pedida");
    comprobar(!terreno.obstaculos.empty(), "el campo tiene obstaculos");
    for (const auto& suministro : terreno.suministros) {
        comprobar(dentro(terreno.limites, suministro.posicion), "suministro dentro de los limites");
        comprobar(!bloqueado(terreno, suministro.posicion), "suministro fuera de toda colision");
    }
}

void prueba_claro_y_obstaculos() {
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    comprobar(dentro(terreno.limites, terreno.inicio), "el inicio esta dentro del campo");
    comprobar(!bloqueado(terreno, terreno.inicio), "el inicio no esta sobre un obstaculo");

    bool claro_libre = true;
    bool dentro_del_campo = true;
    bool separados = true;
    for (std::size_t i = 0; i < terreno.obstaculos.size(); ++i) {
        const auto& obstaculo = terreno.obstaculos[i];
        claro_libre =
            claro_libre && distancia_a_rectangulo(terreno.inicio, obstaculo.colision) >= 64.0F;
        dentro_del_campo =
            dentro_del_campo && obstaculo.colision.origen.x >= 0.0F &&
            obstaculo.colision.origen.y >= 0.0F &&
            obstaculo.colision.origen.x + obstaculo.colision.ancho <= limites.ancho &&
            obstaculo.colision.origen.y + obstaculo.colision.alto <= limites.alto;
        for (std::size_t j = i + 1; j < terreno.obstaculos.size(); ++j) {
            const Rectangulo& a = terreno.obstaculos[i].colision;
            const Rectangulo& b = terreno.obstaculos[j].colision;
            const Rectangulo zona{
                {a.origen.x - 32.0F, a.origen.y - 32.0F}, a.ancho + 64.0F, a.alto + 64.0F};
            separados = separados && !se_intersectan(zona, b);
        }
    }
    comprobar(claro_libre, "el claro de 64 px alrededor del inicio esta libre");
    comprobar(dentro_del_campo, "todas las colisiones caen dentro de los limites");
    comprobar(separados, "queda al menos 32 px libres entre dos obstaculos");
}

void prueba_alcanzables() {
    for (std::uint32_t semilla : {1U, 7U, 42U, 99U}) {
        PlanificadorCampoAbierto planificador(limites, semilla);
        const Terreno terreno = planificador.preparar(caso_mora());
        bool todos = true;
        for (const auto& suministro : terreno.suministros) {
            todos = todos && es_alcanzable(terreno, suministro.posicion);
        }
        comprobar(todos, "todos los suministros se alcanzan rodeando obstaculos (semilla " +
                             std::to_string(semilla) + ")");
    }
}

void prueba_semillas() {
    PlanificadorCampoAbierto fija(limites, 42U);
    const auto primera = firma(fija.preparar(caso_mora()));
    const auto segunda = firma(fija.preparar(caso_mora()));
    comprobar(primera == segunda, "con semilla fija, dos preparar() dan el mismo terreno");

    PlanificadorCampoAbierto gemela(limites, 42U);
    comprobar(firma(gemela.preparar(caso_mora())) == primera,
              "dos planificadores con la misma semilla dan el mismo terreno");

    PlanificadorCampoAbierto otra(limites, 43U);
    comprobar(firma(otra.preparar(caso_mora())) != primera, "otra semilla da otra distribucion");

    PlanificadorCampoAbierto libre(limites);
    std::vector<std::vector<float>> firmas;
    for (int i = 0; i < 6; ++i) {
        firmas.push_back(firma(libre.preparar(caso_mora())));
    }
    const bool hay_variacion = std::any_of(firmas.begin() + 1, firmas.end(),
                                           [&](const auto& f) { return f != firmas.front(); });
    comprobar(hay_variacion, "sin semilla, las distribuciones varian");
}

void prueba_suelo() {
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());
    const auto& suelo = terreno.suelo;

    comprobar(suelo.tamano_pieza == 16.0F, "las piezas de suelo miden 16 px");
    comprobar(suelo.columnas == 100 && suelo.filas == 75, "la cuadricula cubre todo el campo");
    comprobar(suelo.piezas.size() == 100U * 75U, "hay una pieza por celda");

    const auto pieza_en = [&](Punto punto) {
        return suelo.pieza(static_cast<int>(punto.x / suelo.tamano_pieza),
                           static_cast<int>(punto.y / suelo.tamano_pieza));
    };
    comprobar(pieza_en(terreno.inicio).tipo == TipoPiezaSuelo::tierra,
              "el veterinario empieza sobre tierra");
    bool claro_de_tierra = true;
    for (float dx = -64.0F; dx <= 64.0F; dx += 8.0F) {
        for (float dy = -64.0F; dy <= 64.0F; dy += 8.0F) {
            claro_de_tierra =
                claro_de_tierra && pieza_en({terreno.inicio.x + dx, terreno.inicio.y + dy}).tipo ==
                                       TipoPiezaSuelo::tierra;
        }
    }
    comprobar(claro_de_tierra, "el claro de 64 px alrededor del inicio es todo tierra");

    // Cada transicion mira a una tierra segun su giro; el pasto no toca la tierra.
    const int vecinos[4][2] = {
        {0, 1}, {-1, 0}, {0, -1}, {1, 0}}; // giro 0..3: sur, oeste, norte, este
    int tierras = 0;
    int transiciones = 0;
    int pastos = 0;
    bool giros_coherentes = true;
    bool pasto_aislado = true;
    for (int fila = 0; fila < suelo.filas; ++fila) {
        for (int columna = 0; columna < suelo.columnas; ++columna) {
            const PiezaSuelo pieza = suelo.pieza(columna, fila);
            if (pieza.tipo == TipoPiezaSuelo::tierra) {
                ++tierras;
            }

            else if (pieza.tipo == TipoPiezaSuelo::transicion) {
                ++transiciones;
                const bool giro_valido = pieza.giro_cuartos >= 0 && pieza.giro_cuartos < 4;
                giros_coherentes = giros_coherentes && giro_valido &&
                                   suelo.pieza(columna + vecinos[pieza.giro_cuartos % 4][0],
                                               fila + vecinos[pieza.giro_cuartos % 4][1])
                                           .tipo == TipoPiezaSuelo::tierra;
            }

            else {
                ++pastos;
                for (const auto& vecino : vecinos) {
                    pasto_aislado =
                        pasto_aislado && suelo.pieza(columna + vecino[0], fila + vecino[1]).tipo !=
                                             TipoPiezaSuelo::tierra;
                }
            }
        }
    }
    comprobar(tierras > 0 && transiciones > 0 && pastos > tierras,
              "hay pasto, tierra y transicion");
    comprobar(giros_coherentes, "el giro de cada transicion apunta a una pieza de tierra");
    comprobar(pasto_aislado, "ninguna pieza de pasto toca directamente la tierra");
    comprobar(suelo.pieza(-1, 0).tipo == TipoPiezaSuelo::pasto &&
                  suelo.pieza(0, 9999).tipo == TipoPiezaSuelo::pasto,
              "fuera de la cuadricula se devuelve pasto");

    PlanificadorCampoAbierto gemela(limites, 42U);
    comprobar(firma(gemela.preparar(caso_mora())) == firma(terreno),
              "con la misma semilla el suelo es el mismo");
}

void prueba_oleadas() {
    const Punto jugador = limites_centro();
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    const auto gruesas = recorrer(planificador, terreno, 0.5F, 300.0F, jugador);
    comprobar(!gruesas.empty(), "hay apariciones durante los 300 s");
    comprobar(gruesas.front().segundo > 1.0F, "no aparece nadie en el primer segundo");
    comprobar(planificador.avanzar(10.0F, jugador, terreno).empty(),
              "pasado el plan no salen mas apariciones");

    // Un solo paso grande entrega exactamente la misma lista: nada se pierde ni se repite.
    PlanificadorCampoAbierto de_golpe(limites, 42U);
    de_golpe.preparar(caso_mora());
    comprobar(mismas_apariciones(de_golpe.avanzar(300.0F, jugador, terreno), gruesas),
              "pasos de 0.5 s y un paso de 300 s entregan las mismas apariciones");

    // Cada aparicion sale en el paso que le corresponde, ni antes ni mas de un paso despues.
    PlanificadorCampoAbierto fino(limites, 42U);
    fino.preparar(caso_mora());
    const auto finas = recorrer(fino, terreno, 0.25F, 300.0F, jugador);
    bool a_tiempo = finas.size() == gruesas.size();
    for (std::size_t i = 0; a_tiempo && i < finas.size(); ++i) {
        const float retraso = gruesas[i].segundo - finas[i].segundo;
        a_tiempo = retraso >= 0.0F && retraso <= 0.5F;
    }
    comprobar(a_tiempo, "cada aparicion se entrega en el paso que corresponde");

    // Repetir desde preparar() reinicia el reloj y repite el plan.
    planificador.preparar(caso_mora());
    const auto repetidas = recorrer(planificador, terreno, 0.5F, 300.0F, jugador);
    bool igual = repetidas.size() == gruesas.size();
    for (std::size_t i = 0; igual && i < repetidas.size(); ++i) {
        igual = repetidas[i].segundo == gruesas[i].segundo &&
                repetidas[i].aparicion.tipo == gruesas[i].aparicion.tipo;
    }
    comprobar(igual, "preparar() reinicia el reloj de las oleadas");

    // Ritmo del plan.
    bool primer_minuto_solo_animales = true;
    bool contacto_antes_de_tiempo = false;
    bool tirador_antes_de_tiempo = false;
    bool hay_tirador = false;
    for (const Entrega& entrega : gruesas) {
        const TipoEnemigo tipo = entrega.aparicion.tipo;
        if (entrega.segundo < 60.0F && tipo != TipoEnemigo::animal_de_contacto) {
            primer_minuto_solo_animales = false;
        }

        if (entrega.segundo < 60.0F && tipo == TipoEnemigo::superviviente_de_contacto) {
            contacto_antes_de_tiempo = true;
        }

        if (tipo == TipoEnemigo::superviviente_a_distancia) {
            hay_tirador = true;
            tirador_antes_de_tiempo = tirador_antes_de_tiempo || entrega.segundo < 194.0F;
        }
    }
    comprobar(primer_minuto_solo_animales, "el primer minuto solo trae animales de contacto");
    comprobar(!contacto_antes_de_tiempo,
              "los supervivientes de contacto llegan despues del minuto 1");
    comprobar(hay_tirador, "el tramo final trae supervivientes a distancia");
    comprobar(!tirador_antes_de_tiempo, "los supervivientes a distancia solo llegan al final");

    const auto contar = [&](float desde, float hasta) {
        return std::count_if(gruesas.begin(), gruesas.end(), [&](const Entrega& e) {
            return e.segundo > desde && e.segundo <= hasta;
        });
    };
    comprobar(contar(240.0F, 300.0F) > contar(0.0F, 60.0F),
              "el ultimo minuto trae mas enemigos que el primero");
}

// Las apariciones salen fuera de la camara, cerca de ella, por varios lados y en zonas libres.
void prueba_lugar_de_aparicion() {
    const Punto jugador = limites_centro();
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());
    const Rectangulo camara = camara_de(limites, jugador);
    const auto entregas = recorrer(planificador, terreno, 0.5F, 300.0F, jugador);

    bool fuera = true;
    bool cerca = true;
    bool en_el_campo = true;
    bool libres = true;
    int por_lado[4] = {0, 0, 0, 0};
    for (const Entrega& entrega : entregas) {
        const Punto posicion = entrega.aparicion.posicion;
        const float lejania = distancia_a_rectangulo(posicion, camara);
        fuera = fuera && lejania >= 24.0F && lado_de(camara, posicion) >= 0;
        cerca = cerca && lejania <= 160.0F;
        en_el_campo =
            en_el_campo &&
            dentro(Rectangulo{{-1.0F, -1.0F}, limites.ancho + 2.0F, limites.alto + 2.0F}, posicion);
        libres = libres && !bloqueado(terreno, posicion);
        if (lado_de(camara, posicion) >= 0) {
            ++por_lado[lado_de(camara, posicion)];
        }
    }
    comprobar(!entregas.empty(), "hay apariciones que revisar");
    comprobar(fuera, "ninguna aparicion nace a menos de 24 px de la camara");
    comprobar(cerca, "las apariciones nacen cerca de la camara, no al otro lado del campo");
    comprobar(en_el_campo, "las apariciones caen dentro del campo");
    comprobar(libres, "las apariciones no caen sobre obstaculos");

    const int total = static_cast<int>(entregas.size());
    bool varios_lados = true;
    for (int lado : por_lado) {
        varios_lados = varios_lados && lado > 0 && lado * 10 < total * 6;
    }
    comprobar(varios_lados, "aparecen por los cuatro lados, sin que uno concentre casi todo");

    // Con varias semillas, algun tirador aparece junto a un suministro.
    bool tirador_cerca = false;
    for (std::uint32_t semilla = 1; semilla <= 8; ++semilla) {
        PlanificadorCampoAbierto otro(limites, semilla);
        const Terreno otro_terreno = otro.preparar(caso_mora());
        for (const Entrega& entrega : recorrer(otro, otro_terreno, 0.5F, 300.0F, jugador)) {
            if (entrega.aparicion.tipo != TipoEnemigo::superviviente_a_distancia) {
                continue;
            }

            for (const auto& suministro : otro_terreno.suministros) {
                tirador_cerca = tirador_cerca || distancia(entrega.aparicion.posicion,
                                                           suministro.posicion) <= 100.0F;
            }
        }
    }
    comprobar(tirador_cerca, "algun superviviente a distancia aparece cerca de un suministro");
}

// Las apariciones usan la posicion del veterinario en cada paso, no la del inicio.
void prueba_posicion_actual() {
    const Punto esquina{200.0F, 200.0F};
    const Punto centro = limites_centro();
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    const Rectangulo camara_centro = camara_de(limites, centro);
    const Rectangulo camara_esquina = camara_de(limites, esquina);
    bool primera_mitad_correcta = true;
    bool segunda_mitad_correcta = true;
    bool sin_lados_imposibles = true;
    int cuantas = 0;

    for (int paso = 1; paso <= 600; ++paso) {
        const bool segunda_mitad = paso > 300;
        const Punto jugador = segunda_mitad ? esquina : centro;
        const Rectangulo& camara = segunda_mitad ? camara_esquina : camara_centro;
        for (const Aparicion& aparicion : planificador.avanzar(0.5F, jugador, terreno)) {
            ++cuantas;
            const float lejania = distancia_a_rectangulo(aparicion.posicion, camara);
            const bool valida = lejania >= 24.0F && lejania <= 160.0F;
            (segunda_mitad ? segunda_mitad_correcta : primera_mitad_correcta) =
                (segunda_mitad ? segunda_mitad_correcta : primera_mitad_correcta) && valida;
            // Con la camara pegada a la esquina superior izquierda, solo caben derecha y abajo.
            if (segunda_mitad) {
                const int lado = lado_de(camara, aparicion.posicion);
                sin_lados_imposibles = sin_lados_imposibles && (lado == 1 || lado == 2);
            }
        }
    }
    comprobar(cuantas > 0, "hay apariciones en el recorrido con movimiento");
    comprobar(primera_mitad_correcta, "mientras esta en el centro nacen junto a esa camara");
    comprobar(segunda_mitad_correcta, "al moverse, nacen junto a la camara de su nueva posicion");
    comprobar(sin_lados_imposibles, "no nacen fuera del campo cuando la camara toca el borde");
}

// Sin un lugar valido, la aparicion se omite.
void prueba_omision() {
    // Un campo mas chico que la camara: no queda ningun punto fuera de ella dentro del campo.
    const Rectangulo chico{{0.0F, 0.0F}, 600.0F, 300.0F};
    PlanificadorCampoAbierto planificador(chico, 42U);
    const Terreno terreno_chico = planificador.preparar(caso_mora());
    const auto entregas =
        recorrer(planificador, terreno_chico, 0.5F, 300.0F, Punto{300.0F, 150.0F});
    comprobar(entregas.empty(), "si no hay lugar valido se omiten las apariciones");
    comprobar(planificador.avanzar(1.0F, Punto{300.0F, 150.0F}, terreno_chico).empty(),
              "omitir una aparicion no deja pendientes");

    // El campo normal si las entrega.
    PlanificadorCampoAbierto normal(limites, 42U);
    const Terreno terreno_normal = normal.preparar(caso_mora());
    comprobar(!recorrer(normal, terreno_normal, 0.5F, 300.0F, limites_centro()).empty(),
              "en el campo normal si hay apariciones");
}

// La mision pasa el terreno del cuadro: sin suministros ni obstaculos que ya no estan.
void prueba_terreno_recibido() {
    const Punto jugador = limites_centro();
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    // Un muro enorme en la mitad derecha del campo: nada debe nacer dentro.
    Terreno con_muro = terreno;
    const Rectangulo muro{{900.0F, 0.0F}, 700.0F, 1200.0F};
    con_muro.obstaculos.push_back({veterinaria::TipoObstaculo::roca, {900.0F, 1200.0F}, muro});
    con_muro.suministros.clear(); // Ya se recogieron todos.
    const auto entregas = recorrer(planificador, con_muro, 0.5F, 300.0F, jugador);
    bool fuera_del_muro = true;
    for (const Entrega& entrega : entregas) {
        fuera_del_muro = fuera_del_muro && !dentro(muro, entrega.aparicion.posicion);
    }
    comprobar(!entregas.empty(), "con el terreno recibido sigue habiendo apariciones");
    comprobar(fuera_del_muro, "las apariciones respetan los obstaculos del terreno recibido");

    // La version sin posicion usa el terreno de preparar() y el veterinario en el inicio.
    PlanificadorCampoAbierto a(limites, 42U);
    PlanificadorCampoAbierto b(limites, 42U);
    const Terreno ta = a.preparar(caso_mora());
    b.preparar(caso_mora());
    bool equivalentes = true;
    for (int paso = 0; paso < 600; ++paso) {
        const auto sola = b.avanzar(0.5F);
        const auto completa = a.avanzar(0.5F, ta.inicio, ta);
        equivalentes = equivalentes && sola.size() == completa.size();
        for (std::size_t i = 0; equivalentes && i < sola.size(); ++i) {
            equivalentes = sola[i].tipo == completa[i].tipo &&
                           sola[i].posicion.x == completa[i].posicion.x &&
                           sola[i].posicion.y == completa[i].posicion.y;
        }
    }
    comprobar(equivalentes,
              "avanzar(segundos) equivale a avanzar con el inicio y el terreno propios");
}

// El campo real del juego (src/main.cpp) es de 1280 x 900.
void prueba_campo_del_juego() {
    const Rectangulo campo{{0.0F, 0.0F}, 1280.0F, 900.0F};
    PlanificadorCampoAbierto planificador(campo, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());
    comprobar(terreno.suelo.columnas == 80 && terreno.suelo.filas == 57,
              "el suelo del campo de juego es de 80 x 57 piezas");
    comprobar(total_de(terreno, "vendas") == 2 && total_de(terreno, "antiseptico") == 1,
              "el campo de juego ofrece las cantidades pedidas");

    const Punto jugador{terreno.inicio.x, terreno.inicio.y};
    const Rectangulo camara = camara_de(campo, jugador);
    bool todas_bien = true;
    const auto entregas = recorrer(planificador, terreno, 0.5F, 300.0F, jugador);
    for (const Entrega& entrega : entregas) {
        const Punto posicion = entrega.aparicion.posicion;
        todas_bien = todas_bien && distancia_a_rectangulo(posicion, camara) >= 24.0F &&
                     !bloqueado(terreno, posicion);
    }
    comprobar(!entregas.empty() && todas_bien,
              "en el campo de juego las apariciones salen fuera de camara y en zonas libres");
}

} // namespace

int main() {
    prueba_suministros();
    prueba_claro_y_obstaculos();
    prueba_alcanzables();
    prueba_semillas();
    prueba_suelo();
    prueba_oleadas();
    prueba_lugar_de_aparicion();
    prueba_posicion_actual();
    prueba_omision();
    prueba_terreno_recibido();
    prueba_campo_del_juego();

    std::cout << comprobaciones - fallos << " de " << comprobaciones
              << " comprobaciones correctas.\n";
    return fallos == 0 ? 0 : 1;
}