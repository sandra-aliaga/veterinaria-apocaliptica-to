#include "planificador_campo_abierto.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>

namespace veterinaria {

namespace {

// Unidades: pixeles del mundo. Origen arriba a la izquierda, tiempo en segundos.
constexpr float tamano_celda = 16.0F; // Mismo tamano que las piezas de terreno del arte.
constexpr float margen_borde = 16.0F;
constexpr float radio_claro = 64.0F;           // Zona sin obstaculos alrededor del inicio.
constexpr float holgura_jugador = 8.0F;        // Se suma a cada colision al buscar caminos.
constexpr float separacion_obstaculos = 40.0F; // Espacio libre minimo entre dos colisiones.
constexpr float area_por_obstaculo = 96.0F * 96.0F;
constexpr int intentos_por_obstaculo = 30;

constexpr float distancia_minima_suministro = 160.0F;
constexpr float separacion_suministros = 48.0F;

constexpr float duracion_por_defecto = 300.0F;
constexpr float primer_grupo = 5.0F;
constexpr float retraso_entre_enemigos = 0.4F;
constexpr float ancho_camara = 640.0F; // Lienzo de la vista (src/vista/README.md).
constexpr float alto_camara = 360.0F;
constexpr float margen_fuera_de_camara = 24.0F;    // Los enemigos nunca nacen a la vista.
constexpr float profundidad_aparicion = 64.0F;     // Hasta donde se alejan de la camara.
constexpr float alcance_maximo_aparicion = 160.0F; // Ningun enemigo nace mas lejos de la camara.
constexpr float inset_limites = 8.0F;
constexpr float radio_cerca_suministro = 96.0F;
constexpr int intentos_de_posicion = 20;
constexpr float pi = 3.14159265F;

constexpr int radio_parche_inicial = 6; // Piezas de tierra a cada lado del inicio.
constexpr float area_por_parche = 400.0F * 400.0F;
constexpr int holgura_entre_parches = 3; // Piezas libres entre dos claros de tierra.

// ---------------------------------------------------------------------------
// Aleatoriedad. Se evitan las distribuciones de <random> porque su resultado
// cambia entre compiladores; mt19937 si es igual en todos.
// ---------------------------------------------------------------------------

float aleatorio_unidad(std::mt19937& generador) {
    return static_cast<float>(generador() >> 8) / 16777216.0F; // [0, 1)
}

std::size_t indice_aleatorio(std::mt19937& generador, std::size_t cantidad) {
    return static_cast<std::size_t>(generador() % cantidad);
}

template <typename T> void barajar(std::vector<T>& elementos, std::mt19937& generador) {
    for (std::size_t i = elementos.size(); i > 1; --i) {
        std::swap(elementos[i - 1], elementos[indice_aleatorio(generador, i)]);
    }
}

// ---------------------------------------------------------------------------
// Geometria
// ---------------------------------------------------------------------------

float distancia(Punto a, Punto b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

bool se_intersectan(const Rectangulo& a, const Rectangulo& b) {
    return a.origen.x < b.origen.x + b.ancho && b.origen.x < a.origen.x + a.ancho &&
           a.origen.y < b.origen.y + b.alto && b.origen.y < a.origen.y + a.alto;
}

Rectangulo inflar(const Rectangulo& rectangulo, float margen) {
    return {{rectangulo.origen.x - margen, rectangulo.origen.y - margen},
            rectangulo.ancho + 2.0F * margen,
            rectangulo.alto + 2.0F * margen};
}

bool contiene(const Rectangulo& rectangulo, Punto punto) {
    return punto.x > rectangulo.origen.x && punto.x < rectangulo.origen.x + rectangulo.ancho &&
           punto.y > rectangulo.origen.y && punto.y < rectangulo.origen.y + rectangulo.alto;
}

float distancia_a_rectangulo(Punto punto, const Rectangulo& rectangulo) {
    const float dx = std::max(
        {rectangulo.origen.x - punto.x, 0.0F, punto.x - (rectangulo.origen.x + rectangulo.ancho)});
    const float dy = std::max(
        {rectangulo.origen.y - punto.y, 0.0F, punto.y - (rectangulo.origen.y + rectangulo.alto)});
    return std::hypot(dx, dy);
}

bool dentro_de_obstaculo(Punto punto, const std::vector<ObstaculoMapa>& obstaculos) {
    return std::any_of(obstaculos.begin(), obstaculos.end(), [&](const ObstaculoMapa& obstaculo) {
        return contiene(inflar(obstaculo.colision, holgura_jugador), punto);
    });
}

bool dentro_de_limites(const Rectangulo& limites, Punto punto, float margen) {
    return punto.x >= limites.origen.x + margen &&
           punto.x <= limites.origen.x + limites.ancho - margen &&
           punto.y >= limites.origen.y + margen &&
           punto.y <= limites.origen.y + limites.alto - margen;
}

// ---------------------------------------------------------------------------
// Obstaculos
// ---------------------------------------------------------------------------

struct Medidas {
    float ancho;
    float alto;
};

// Medidas de la zona solida. En el arbol es la base del tronco; la copa se dibuja encima.
Medidas medidas_de(TipoObstaculo tipo) {
    switch (tipo) {
    case TipoObstaculo::arbol:
        return {16.0F, 12.0F};
    case TipoObstaculo::roca:
        return {24.0F, 16.0F};
    case TipoObstaculo::tronco:
        return {32.0F, 12.0F};
    }
    return {16.0F, 12.0F};
}

TipoObstaculo elegir_tipo(std::mt19937& generador) {
    const float azar = aleatorio_unidad(generador);
    if (azar < 0.5F) {
        return TipoObstaculo::arbol;
    }
    return azar < 0.8F ? TipoObstaculo::roca : TipoObstaculo::tronco;
}

std::vector<ObstaculoMapa> generar_obstaculos(const Rectangulo& limites, Punto inicio,
                                              std::mt19937& generador) {
    std::vector<ObstaculoMapa> obstaculos;
    const int objetivo = static_cast<int>(limites.ancho * limites.alto / area_por_obstaculo);
    const int intentos = objetivo * intentos_por_obstaculo;

    for (int intento = 0; intento < intentos && static_cast<int>(obstaculos.size()) < objetivo;
         ++intento) {
        const TipoObstaculo tipo = elegir_tipo(generador);
        const Medidas medidas = medidas_de(tipo);
        const float libre_x = limites.ancho - 2.0F * margen_borde - medidas.ancho;
        const float libre_y = limites.alto - 2.0F * margen_borde - medidas.alto;
        if (libre_x <= 0.0F || libre_y <= 0.0F) {
            continue;
        }

        const Rectangulo colision{
            {limites.origen.x + margen_borde + aleatorio_unidad(generador) * libre_x,
             limites.origen.y + margen_borde + aleatorio_unidad(generador) * libre_y},
            medidas.ancho,
            medidas.alto};

        if (distancia_a_rectangulo(inicio, colision) < radio_claro) {
            continue;
        }
        const Rectangulo zona = inflar(colision, separacion_obstaculos);
        const bool estorba =
            std::any_of(obstaculos.begin(), obstaculos.end(), [&](const ObstaculoMapa& otro) {
                return se_intersectan(zona, otro.colision);
            });
        if (estorba) {
            continue;
        }

        // El anclaje es el centro de la base de la colision, donde se apoya el sprite.
        const Punto anclaje{colision.origen.x + colision.ancho / 2.0F,
                            colision.origen.y + colision.alto};
        obstaculos.push_back(ObstaculoMapa{tipo, anclaje, colision});
    }

    // Orden de arriba hacia abajo: la vista puede dibujar sin reordenar.
    std::stable_sort(
        obstaculos.begin(), obstaculos.end(),
        [](const ObstaculoMapa& a, const ObstaculoMapa& b) { return a.anclaje.y < b.anclaje.y; });
    return obstaculos;
}

// ---------------------------------------------------------------------------
// Rejilla de alcanzabilidad
// ---------------------------------------------------------------------------

struct Rejilla {
    Rectangulo limites;
    int columnas;
    int filas;
    std::vector<char> alcanzable;

    int indice(int columna, int fila) const {
        return fila * columnas + columna;
    }

    Punto centro(int columna, int fila) const {
        return {limites.origen.x + (static_cast<float>(columna) + 0.5F) * tamano_celda,
                limites.origen.y + (static_cast<float>(fila) + 0.5F) * tamano_celda};
    }
};

int limitar(int valor, int minimo, int maximo) {
    return std::min(std::max(valor, minimo), maximo);
}

// Marca con BFS (4 vecinos) las celdas a las que se llega desde el inicio sin cruzar colisiones.
Rejilla construir_rejilla(const Rectangulo& limites, const std::vector<ObstaculoMapa>& obstaculos,
                          Punto inicio) {
    Rejilla rejilla{limites,
                    std::max(1, static_cast<int>(std::ceil(limites.ancho / tamano_celda))),
                    std::max(1, static_cast<int>(std::ceil(limites.alto / tamano_celda))),
                    {}};
    const auto total = static_cast<std::size_t>(rejilla.columnas) * rejilla.filas;
    std::vector<char> bloqueada(total, 0);
    rejilla.alcanzable.assign(total, 0);

    for (const ObstaculoMapa& obstaculo : obstaculos) {
        const Rectangulo zona = inflar(obstaculo.colision, holgura_jugador);
        const int c0 = limitar(
            static_cast<int>(std::floor((zona.origen.x - limites.origen.x) / tamano_celda)) - 1, 0,
            rejilla.columnas - 1);
        const int c1 =
            limitar(static_cast<int>(std::floor((zona.origen.x + zona.ancho - limites.origen.x) /
                                                tamano_celda)) +
                        1,
                    0, rejilla.columnas - 1);
        const int f0 = limitar(
            static_cast<int>(std::floor((zona.origen.y - limites.origen.y) / tamano_celda)) - 1, 0,
            rejilla.filas - 1);
        const int f1 = limitar(static_cast<int>(std::floor(
                                   (zona.origen.y + zona.alto - limites.origen.y) / tamano_celda)) +
                                   1,
                               0, rejilla.filas - 1);
        for (int fila = f0; fila <= f1; ++fila) {
            for (int columna = c0; columna <= c1; ++columna) {
                if (contiene(zona, rejilla.centro(columna, fila))) {
                    bloqueada[static_cast<std::size_t>(rejilla.indice(columna, fila))] = 1;
                }
            }
        }
    }

    const int columna_inicio = limitar(
        static_cast<int>((inicio.x - limites.origen.x) / tamano_celda), 0, rejilla.columnas - 1);
    const int fila_inicio = limitar(static_cast<int>((inicio.y - limites.origen.y) / tamano_celda),
                                    0, rejilla.filas - 1);

    std::vector<int> cola;
    const int primera = rejilla.indice(columna_inicio, fila_inicio);
    rejilla.alcanzable[static_cast<std::size_t>(primera)] = 1;
    cola.push_back(primera);

    constexpr int pasos[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (std::size_t cabeza = 0; cabeza < cola.size(); ++cabeza) {
        const int columna = cola[cabeza] % rejilla.columnas;
        const int fila = cola[cabeza] / rejilla.columnas;
        for (const auto& paso : pasos) {
            const int c = columna + paso[0];
            const int f = fila + paso[1];
            if (c < 0 || c >= rejilla.columnas || f < 0 || f >= rejilla.filas) {
                continue;
            }
            const auto vecina = static_cast<std::size_t>(rejilla.indice(c, f));
            if (bloqueada[vecina] != 0 || rejilla.alcanzable[vecina] != 0) {
                continue;
            }
            rejilla.alcanzable[vecina] = 1;
            cola.push_back(static_cast<int>(vecina));
        }
    }
    return rejilla;
}

// ---------------------------------------------------------------------------
// Suministros
// ---------------------------------------------------------------------------

std::vector<Punto> celdas_candidatas(const Rejilla& rejilla, Punto inicio, float minima,
                                     float maxima) {
    std::vector<Punto> candidatas;
    for (int fila = 0; fila < rejilla.filas; ++fila) {
        for (int columna = 0; columna < rejilla.columnas; ++columna) {
            if (rejilla.alcanzable[static_cast<std::size_t>(rejilla.indice(columna, fila))] == 0) {
                continue;
            }
            const Punto centro = rejilla.centro(columna, fila);
            const float lejania = distancia(centro, inicio);
            if (lejania >= minima && lejania <= maxima &&
                dentro_de_limites(rejilla.limites, centro, margen_borde)) {
                candidatas.push_back(centro);
            }
        }
    }
    return candidatas;
}

// Una entrada por unidad pedida: asi las unidades quedan repartidas en posiciones distintas.
std::vector<SuministroEnMapa> colocar_suministros(const PropuestaMision& propuesta,
                                                  const Rejilla& rejilla, Punto inicio,
                                                  std::mt19937& generador) {
    const Rectangulo& limites = rejilla.limites;
    const float maxima =
        std::max(distancia_minima_suministro + 80.0F, 0.4F * std::min(limites.ancho, limites.alto));

    std::vector<Punto> candidatas =
        celdas_candidatas(rejilla, inicio, distancia_minima_suministro, maxima);
    if (candidatas.empty()) {
        candidatas = celdas_candidatas(rejilla, inicio, tamano_celda,
                                       std::numeric_limits<float>::infinity());
    }

    if (candidatas.empty()) {
        candidatas.push_back(inicio);
    }
    barajar(candidatas, generador);

    std::vector<SuministroEnMapa> colocados;
    for (const CantidadSuministro& requerido : propuesta.requeridos) {
        for (int unidad = 0; unidad < requerido.cantidad; ++unidad) {
            const auto separada = [&](Punto candidata) {
                return std::all_of(
                    colocados.begin(), colocados.end(), [&](const SuministroEnMapa& otro) {
                        return distancia(otro.posicion, candidata) >= separacion_suministros;
                    });
            };
            const auto elegida = std::find_if(candidatas.begin(), candidatas.end(), separada);
            const Punto posicion = elegida != candidatas.end()
                                       ? *elegida
                                       : candidatas[colocados.size() % candidatas.size()];
            colocados.push_back(SuministroEnMapa{CantidadSuministro{requerido.id, 1}, posicion});
        }
    }
    return colocados;
}

// ---------------------------------------------------------------------------
// Suelo
// ---------------------------------------------------------------------------

struct Parche {
    int columna;
    int fila;
    int ancho;
    int alto;
};

bool parches_se_tocan(const Parche& a, const Parche& b, int holgura) {
    return a.columna < b.columna + b.ancho + holgura && b.columna < a.columna + a.ancho + holgura &&
           a.fila < b.fila + b.alto + holgura && b.fila < a.fila + a.alto + holgura;
}

// Claros de tierra rectangulares: uno bajo el inicio y otros repartidos por el campo. Alrededor
// de cada claro va un anillo de transiciones; el pasto cubre el resto.
SueloMapa generar_suelo(const Rectangulo& limites, Punto inicio, std::mt19937& generador) {
    SueloMapa suelo;
    suelo.tamano_pieza = tamano_celda;
    suelo.columnas = std::max(1, static_cast<int>(std::ceil(limites.ancho / tamano_celda)));
    suelo.filas = std::max(1, static_cast<int>(std::ceil(limites.alto / tamano_celda)));
    const int columnas = suelo.columnas;
    const int filas = suelo.filas;
    suelo.piezas.assign(static_cast<std::size_t>(columnas) * static_cast<std::size_t>(filas),
                        PiezaSuelo{});

    const int columna_inicio =
        limitar(static_cast<int>((inicio.x - limites.origen.x) / tamano_celda), 0, columnas - 1);
    const int fila_inicio =
        limitar(static_cast<int>((inicio.y - limites.origen.y) / tamano_celda), 0, filas - 1);
    std::vector<Parche> parches{Parche{columna_inicio - radio_parche_inicial,
                                       fila_inicio - radio_parche_inicial, 2 * radio_parche_inicial,
                                       2 * radio_parche_inicial}};

    const int objetivo = static_cast<int>(limites.ancho * limites.alto / area_por_parche);
    for (int intento = 0; intento < objetivo * 20 && static_cast<int>(parches.size()) <= objetivo;
         ++intento) {
        const int ancho = 4 + static_cast<int>(generador() % 7);
        const int alto = 4 + static_cast<int>(generador() % 7);
        const int libre_columnas = columnas - ancho - 2;
        const int libre_filas = filas - alto - 2;
        if (libre_columnas <= 0 || libre_filas <= 0) {
            continue;
        }
        const Parche parche{
            1 + static_cast<int>(generador() % static_cast<unsigned>(libre_columnas)),
            1 + static_cast<int>(generador() % static_cast<unsigned>(libre_filas)), ancho, alto};
        const bool estorba = std::any_of(parches.begin(), parches.end(), [&](const Parche& otro) {
            return parches_se_tocan(parche, otro, holgura_entre_parches);
        });
        if (!estorba) {
            parches.push_back(parche);
        }
    }

    std::vector<char> tierra(suelo.piezas.size(), 0);
    for (const Parche& parche : parches) {
        for (int fila = std::max(0, parche.fila); fila < std::min(filas, parche.fila + parche.alto);
             ++fila) {
            for (int columna = std::max(0, parche.columna);
                 columna < std::min(columnas, parche.columna + parche.ancho); ++columna) {
                tierra[static_cast<std::size_t>(fila * columnas + columna)] = 1;
            }
        }
    }

    const auto es_tierra = [&](int columna, int fila) {
        return columna >= 0 && columna < columnas && fila >= 0 && fila < filas &&
               tierra[static_cast<std::size_t>(fila * columnas + columna)] != 0;
    };
    for (int fila = 0; fila < filas; ++fila) {
        for (int columna = 0; columna < columnas; ++columna) {
            PiezaSuelo& pieza = suelo.piezas[static_cast<std::size_t>(fila * columnas + columna)];
            if (es_tierra(columna, fila)) {
                pieza.tipo = TipoPiezaSuelo::tierra;
            }

            else if (es_tierra(columna, fila + 1)) {
                pieza = {TipoPiezaSuelo::transicion, 0}; // Tierra al sur: el PNG tal cual.
            }

            else if (es_tierra(columna - 1, fila)) {
                pieza = {TipoPiezaSuelo::transicion, 1}; // Tierra al oeste.
            }

            else if (es_tierra(columna, fila - 1)) {
                pieza = {TipoPiezaSuelo::transicion, 2}; // Tierra al norte.
            }

            else if (es_tierra(columna + 1, fila)) {
                pieza = {TipoPiezaSuelo::transicion, 3}; // Tierra al este.
            }
        }
    }
    return suelo;
}

// ---------------------------------------------------------------------------
// Oleadas
// ---------------------------------------------------------------------------

enum class Fase { dispersa, mixta, densa };

Fase fase_en(float segundo, float duracion) {
    if (segundo < duracion / 5.0F) {
        return Fase::dispersa; // Primer minuto de 300 s.
    }
    return segundo < duracion * 13.0F / 20.0F ? Fase::mixta : Fase::densa;
}

float intervalo_de(Fase fase) {
    switch (fase) {
    case Fase::dispersa:
        return 12.0F;
    case Fase::mixta:
        return 10.0F;
    case Fase::densa:
        return 6.0F;
    }
    return 12.0F;
}

int tamano_de_grupo(Fase fase, std::mt19937& generador) {
    switch (fase) {
    case Fase::dispersa:
        return 2 + static_cast<int>(generador() % 2); // 2 a 3
    case Fase::mixta:
        return 3 + static_cast<int>(generador() % 2); // 3 a 4
    case Fase::densa:
        return 4 + static_cast<int>(generador() % 3); // 4 a 6
    }
    return 2;
}

TipoEnemigo tipo_de_miembro(Fase fase, int posicion) {
    switch (fase) {
    case Fase::dispersa:
        return TipoEnemigo::animal_de_contacto;
    case Fase::mixta:
        return posicion == 0 ? TipoEnemigo::superviviente_de_contacto
                             : TipoEnemigo::animal_de_contacto;
    case Fase::densa:
        if (posicion == 0 || posicion == 4) {
            return TipoEnemigo::superviviente_a_distancia;
        }
        return posicion == 1 ? TipoEnemigo::superviviente_de_contacto
                             : TipoEnemigo::animal_de_contacto;
    }
    return TipoEnemigo::animal_de_contacto;
}

struct EnemigoPlanificado {
    float segundo;
    TipoEnemigo tipo;
    int lado;
};

// La camara se centra en el veterinario y no sale del campo (igual que la vista).
Rectangulo camara_en(const Rectangulo& limites, Punto jugador) {
    const float x_maxima = limites.origen.x + std::max(0.0F, limites.ancho - ancho_camara);
    const float y_maxima = limites.origen.y + std::max(0.0F, limites.alto - alto_camara);
    return {{std::clamp(jugador.x - ancho_camara / 2.0F, limites.origen.x, x_maxima),
             std::clamp(jugador.y - alto_camara / 2.0F, limites.origen.y, y_maxima)},
            ancho_camara,
            alto_camara};
}

bool fuera_de_camara(const Rectangulo& camara, Punto punto) {
    const Rectangulo ampliada = inflar(camara, margen_fuera_de_camara);
    return punto.x < ampliada.origen.x || punto.x > ampliada.origen.x + ampliada.ancho ||
           punto.y < ampliada.origen.y || punto.y > ampliada.origen.y + ampliada.alto;
}

bool posicion_libre(const Terreno& terreno, Punto punto) {
    return dentro_de_limites(terreno.limites, punto, inset_limites) &&
           !dentro_de_obstaculo(punto, terreno.obstaculos);
}

// Lados: 0 arriba, 1 derecha, 2 abajo, 3 izquierda. El punto queda justo fuera de la camara.
Punto punto_junto_a_camara(const Rectangulo& camara, int lado, std::mt19937& generador) {
    const float fondo =
        margen_fuera_de_camara + 4.0F + aleatorio_unidad(generador) * profundidad_aparicion;
    const float a = aleatorio_unidad(generador);
    switch (lado) {
    case 0:
        return {camara.origen.x + a * camara.ancho, camara.origen.y - fondo};
    case 1:
        return {camara.origen.x + camara.ancho + fondo, camara.origen.y + a * camara.alto};
    case 2:
        return {camara.origen.x + a * camara.ancho, camara.origen.y + camara.alto + fondo};
    default:
        return {camara.origen.x - fondo, camara.origen.y + a * camara.alto};
    }
}

// Los tiradores prefieren un lugar cerca de un suministro, pero siempre junto a la camara.
std::optional<Punto> punto_cerca_de_suministro(const Terreno& terreno, const Rectangulo& camara,
                                               std::mt19937& generador) {
    if (terreno.suministros.empty()) {
        return std::nullopt;
    }

    for (int intento = 0; intento < intentos_de_posicion; ++intento) {
        const Punto centro =
            terreno.suministros[indice_aleatorio(generador, terreno.suministros.size())].posicion;
        const float angulo = aleatorio_unidad(generador) * 2.0F * pi;
        const float radio = 32.0F + aleatorio_unidad(generador) * (radio_cerca_suministro - 32.0F);
        const Punto punto{centro.x + std::cos(angulo) * radio, centro.y + std::sin(angulo) * radio};
        if (fuera_de_camara(camara, punto) &&
            distancia_a_rectangulo(punto, camara) <= alcance_maximo_aparicion &&
            posicion_libre(terreno, punto)) {
            return punto;
        }
    }
    return std::nullopt;
}

// Prueba primero el lado previsto y luego los demas. Sin lugar valido devuelve nada.
std::optional<Punto> buscar_posicion(const Terreno& terreno, Punto jugador, TipoEnemigo tipo,
                                     int lado_preferido, std::mt19937& generador) {
    const Rectangulo camara = camara_en(terreno.limites, jugador);

    if (tipo == TipoEnemigo::superviviente_a_distancia) {
        if (const auto cerca = punto_cerca_de_suministro(terreno, camara, generador)) {
            return cerca;
        }
    }

    for (int giro = 0; giro < 4; ++giro) {
        const int lado = (lado_preferido + giro) % 4;
        for (int intento = 0; intento < intentos_de_posicion; ++intento) {
            const Punto punto = punto_junto_a_camara(camara, lado, generador);
            if (posicion_libre(terreno, punto)) {
                return punto;
            }
        }
    }
    return std::nullopt;
}

// Solo fija cuando sale cada enemigo y por que lado prefiere entrar. Dentro de un grupo, cada
// miembro recibe un lado distinto mientras alcancen los cuatro.
std::vector<EnemigoPlanificado> planificar_oleadas(int limite_segundos, std::mt19937& generador) {
    const float duracion =
        limite_segundos > 0 ? static_cast<float>(limite_segundos) : duracion_por_defecto;
    std::vector<EnemigoPlanificado> plan;

    for (float segundo = primer_grupo; segundo < duracion;) {
        const Fase fase = fase_en(segundo, duracion);
        const int tamano = tamano_de_grupo(fase, generador);
        std::vector<int> lados{0, 1, 2, 3};
        barajar(lados, generador);
        for (int miembro = 0; miembro < tamano; ++miembro) {
            plan.push_back(EnemigoPlanificado{
                segundo + static_cast<float>(miembro) * retraso_entre_enemigos,
                tipo_de_miembro(fase, miembro), lados[static_cast<std::size_t>(miembro % 4)]});
        }
        segundo += intervalo_de(fase);
    }
    return plan;
}

} // namespace

PlanificadorCampoAbierto::PlanificadorCampoAbierto(Rectangulo limites,
                                                   std::optional<std::uint32_t> semilla_fija)
    : limites_(limites), semilla_fija_(semilla_fija),
      generador_(semilla_fija.value_or(std::mt19937::default_seed)), segundos_transcurridos_(0.0F),
      terreno_{}, siguiente_aparicion_(0) {
}

Terreno PlanificadorCampoAbierto::preparar(const PropuestaMision& propuesta) {
    if (semilla_fija_) {
        generador_.seed(*semilla_fija_);
    }

    else {
        std::random_device dispositivo;
        generador_.seed(dispositivo());
    }
    segundos_transcurridos_ = 0.0F;
    siguiente_aparicion_ = 0;
    oleadas_.clear();

    Terreno terreno;
    terreno.limites = limites_;
    terreno.inicio = {limites_.origen.x + limites_.ancho / 2.0F,
                      limites_.origen.y + limites_.alto / 2.0F};
    terreno.obstaculos = generar_obstaculos(limites_, terreno.inicio, generador_);

    const Rejilla rejilla = construir_rejilla(limites_, terreno.obstaculos, terreno.inicio);
    terreno.suministros = colocar_suministros(propuesta, rejilla, terreno.inicio, generador_);

    // Las oleadas consultan obstaculos y suministros al vencer; el suelo no les hace falta.
    terreno_ = terreno;
    terreno.suelo = generar_suelo(limites_, terreno.inicio, generador_);

    for (const EnemigoPlanificado& enemigo :
         planificar_oleadas(propuesta.limite_segundos, generador_)) {
        oleadas_.push_back(AparicionProgramada{enemigo.segundo, enemigo.tipo, enemigo.lado});
    }
    return terreno;
}

std::vector<Aparicion> PlanificadorCampoAbierto::avanzar(float segundos) {
    // Sin datos del cuadro, se usa el terreno de preparar() y el veterinario en el inicio.
    return avanzar(segundos, terreno_.inicio, terreno_);
}

std::vector<Aparicion> PlanificadorCampoAbierto::avanzar(float segundos, Punto jugador,
                                                         const Terreno& terreno) {
    if (segundos > 0.0F) {
        segundos_transcurridos_ += segundos;
    }
    std::vector<Aparicion> vencidas;
    while (siguiente_aparicion_ < oleadas_.size() &&
           oleadas_[siguiente_aparicion_].segundo <= segundos_transcurridos_) {
        const AparicionProgramada& programada = oleadas_[siguiente_aparicion_];
        ++siguiente_aparicion_;

        // Sin un lugar valido, esta aparicion se omite.
        if (const auto posicion =
                buscar_posicion(terreno, jugador, programada.tipo, programada.lado, generador_)) {
            vencidas.push_back(Aparicion{programada.tipo, *posicion});
        }
    }
    return vencidas;
}

} // namespace veterinaria
