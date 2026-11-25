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
using veterinaria::PlanificadorCampoAbierto;
using veterinaria::PropuestaMision;
using veterinaria::Punto;
using veterinaria::Rectangulo;
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

const Rectangulo limites{{0.0F, 0.0F}, 1600.0F, 1200.0F};

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

struct Entrega {
    float segundo; // Final del paso en que se entrego.
    Aparicion aparicion;
};

std::vector<Entrega> recorrer(PlanificadorCampoAbierto& planificador, float paso, float total) {
    std::vector<Entrega> entregas;
    const int pasos = static_cast<int>(total / paso);
    for (int i = 1; i <= pasos; ++i) {
        for (const Aparicion& aparicion : planificador.avanzar(paso)) {
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

void prueba_oleadas() {
    PlanificadorCampoAbierto planificador(limites, 42U);
    const Terreno terreno = planificador.preparar(caso_mora());

    const auto gruesas = recorrer(planificador, 0.5F, 300.0F);
    comprobar(!gruesas.empty(), "hay apariciones durante los 300 s");
    comprobar(gruesas.front().segundo > 1.0F, "no aparece nadie en el primer segundo");
    comprobar(planificador.avanzar(10.0F).empty(), "pasado el plan no salen mas apariciones");

    // Un solo paso grande entrega exactamente la misma lista: nada se pierde ni se repite.
    PlanificadorCampoAbierto de_golpe(limites, 42U);
    de_golpe.preparar(caso_mora());
    comprobar(mismas_apariciones(de_golpe.avanzar(300.0F), gruesas),
              "pasos de 0.5 s y un paso de 300 s entregan las mismas apariciones");

    // Cada aparicion sale en el paso que le corresponde, ni antes ni mas de un paso despues.
    PlanificadorCampoAbierto fino(limites, 42U);
    fino.preparar(caso_mora());
    const auto finas = recorrer(fino, 0.25F, 300.0F);
    bool a_tiempo = finas.size() == gruesas.size();
    for (std::size_t i = 0; a_tiempo && i < finas.size(); ++i) {
        const float retraso = gruesas[i].segundo - finas[i].segundo;
        a_tiempo = retraso >= 0.0F && retraso <= 0.5F;
    }
    comprobar(a_tiempo, "cada aparicion se entrega en el paso que corresponde");

    // Repetir desde preparar() reinicia el reloj y repite el plan.
    planificador.preparar(caso_mora());
    const auto repetidas = recorrer(planificador, 0.5F, 300.0F);
    bool igual = repetidas.size() == gruesas.size();
    for (std::size_t i = 0; igual && i < repetidas.size(); ++i) {
        igual = repetidas[i].segundo == gruesas[i].segundo &&
                repetidas[i].aparicion.tipo == gruesas[i].aparicion.tipo;
    }
    comprobar(igual, "preparar() reinicia el reloj de las oleadas");

    // Ritmo del plan y lugares de aparicion.
    bool primer_minuto_solo_animales = true;
    bool contacto_antes_de_tiempo = false;
    bool tirador_antes_de_tiempo = false;
    bool hay_tirador = false;
    bool tirador_cerca_de_suministro = false;
    bool lejos_del_veterinario = true;
    bool en_el_campo = true;
    bool libres = true;
    for (const Entrega& entrega : gruesas) {
        const TipoEnemigo tipo = entrega.aparicion.tipo;
        const Punto posicion = entrega.aparicion.posicion;
        if (entrega.segundo < 60.0F && tipo != TipoEnemigo::animal_de_contacto) {
            primer_minuto_solo_animales = false;
        }

        if (entrega.segundo < 60.0F && tipo == TipoEnemigo::superviviente_de_contacto) {
            contacto_antes_de_tiempo = true;
        }

        if (tipo == TipoEnemigo::superviviente_a_distancia) {
            hay_tirador = true;
            tirador_antes_de_tiempo = tirador_antes_de_tiempo || entrega.segundo < 194.0F;
            for (const auto& suministro : terreno.suministros) {
                tirador_cerca_de_suministro = tirador_cerca_de_suministro ||
                                              distancia(posicion, suministro.posicion) <= 120.0F;
            }
        }
        lejos_del_veterinario =
            lejos_del_veterinario && distancia(posicion, terreno.inicio) >= 100.0F;
        en_el_campo =
            en_el_campo &&
            dentro(Rectangulo{{-1.0F, -1.0F}, limites.ancho + 2.0F, limites.alto + 2.0F}, posicion);
        libres = libres && !bloqueado(terreno, posicion);
    }
    comprobar(primer_minuto_solo_animales, "el primer minuto solo trae animales de contacto");
    comprobar(!contacto_antes_de_tiempo,
              "los supervivientes de contacto llegan despues del minuto 1");
    comprobar(hay_tirador, "el tramo final trae supervivientes a distancia");
    comprobar(!tirador_antes_de_tiempo, "los supervivientes a distancia solo llegan al final");
    comprobar(tirador_cerca_de_suministro,
              "algun superviviente a distancia aparece cerca de un suministro");
    comprobar(lejos_del_veterinario, "las apariciones dejan espacio alrededor del veterinario");
    comprobar(en_el_campo, "las apariciones caen dentro del campo");
    comprobar(libres, "las apariciones no caen sobre obstaculos");

    // Las oleadas se hacen mas densas con el tiempo.
    const auto contar = [&](float desde, float hasta) {
        return std::count_if(gruesas.begin(), gruesas.end(), [&](const Entrega& e) {
            return e.segundo > desde && e.segundo <= hasta;
        });
    };
    comprobar(contar(240.0F, 300.0F) > contar(0.0F, 60.0F),
              "el ultimo minuto trae mas enemigos que el primero");
}

} // namespace

int main() {
    prueba_suministros();
    prueba_claro_y_obstaculos();
    prueba_alcanzables();
    prueba_semillas();
    prueba_oleadas();

    std::cout << comprobaciones - fallos << " de " << comprobaciones
              << " comprobaciones correctas.\n";
    return fallos == 0 ? 0 : 1;
}