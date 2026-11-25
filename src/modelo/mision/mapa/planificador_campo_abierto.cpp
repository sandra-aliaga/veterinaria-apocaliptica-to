#include "planificador_campo_abierto.h"

#include <algorithm>
#include <cmath>
#include <limits>
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
constexpr float distancia_minima_enemigos = 160.0F; // Espacio libre alrededor del veterinario.
constexpr float inset_perimetro = 8.0F;
constexpr float radio_cerca_suministro = 96.0F;
constexpr int intentos_de_posicion = 20;
constexpr float pi = 3.14159265F;

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

bool posicion_valida_para_enemigo(const Terreno& terreno, Punto punto) {
    return distancia(punto, terreno.inicio) >= distancia_minima_enemigos &&
           !dentro_de_obstaculo(punto, terreno.obstaculos);
}

Punto punto_en_perimetro(const Terreno& terreno, std::mt19937& generador) {
    const Rectangulo& limites = terreno.limites;
    Punto punto{limites.origen.x, limites.origen.y};
    for (int intento = 0; intento < intentos_de_posicion; ++intento) {
        const float a = aleatorio_unidad(generador);
        const float x =
            limites.origen.x + inset_perimetro + a * (limites.ancho - 2.0F * inset_perimetro);
        const float y =
            limites.origen.y + inset_perimetro + a * (limites.alto - 2.0F * inset_perimetro);
        switch (generador() % 4) {
        case 0:
            punto = {x, limites.origen.y + inset_perimetro};
            break;
        case 1:
            punto = {x, limites.origen.y + limites.alto - inset_perimetro};
            break;
        case 2:
            punto = {limites.origen.x + inset_perimetro, y};
            break;
        default:
            punto = {limites.origen.x + limites.ancho - inset_perimetro, y};
            break;
        }
        if (posicion_valida_para_enemigo(terreno, punto)) {
            return punto;
        }
    }
    return punto;
}

// Los tiradores aparecen cerca de un suministro; si no hay sitio, en el perimetro.
Punto punto_cerca_de_suministro(const Terreno& terreno, std::mt19937& generador) {
    if (!terreno.suministros.empty()) {
        for (int intento = 0; intento < intentos_de_posicion; ++intento) {
            const Punto centro =
                terreno.suministros[indice_aleatorio(generador, terreno.suministros.size())]
                    .posicion;
            const float angulo = aleatorio_unidad(generador) * 2.0F * pi;
            const float radio =
                32.0F + aleatorio_unidad(generador) * (radio_cerca_suministro - 32.0F);
            const Punto punto{centro.x + std::cos(angulo) * radio,
                              centro.y + std::sin(angulo) * radio};
            if (dentro_de_limites(terreno.limites, punto, inset_perimetro) &&
                posicion_valida_para_enemigo(terreno, punto)) {
                return punto;
            }
        }
    }
    return punto_en_perimetro(terreno, generador);
}

std::vector<std::pair<float, Aparicion>>
planificar_oleadas(int limite_segundos, const Terreno& terreno, std::mt19937& generador) {
    const float duracion =
        limite_segundos > 0 ? static_cast<float>(limite_segundos) : duracion_por_defecto;
    std::vector<std::pair<float, Aparicion>> plan;

    for (float segundo = primer_grupo; segundo < duracion;) {
        const Fase fase = fase_en(segundo, duracion);
        const int tamano = tamano_de_grupo(fase, generador);
        for (int miembro = 0; miembro < tamano; ++miembro) {
            const TipoEnemigo tipo = tipo_de_miembro(fase, miembro);
            const Punto posicion = tipo == TipoEnemigo::superviviente_a_distancia
                                       ? punto_cerca_de_suministro(terreno, generador)
                                       : punto_en_perimetro(terreno, generador);
            plan.emplace_back(segundo + static_cast<float>(miembro) * retraso_entre_enemigos,
                              Aparicion{tipo, posicion});
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
      siguiente_aparicion_(0) {
}

Terreno PlanificadorCampoAbierto::preparar(const PropuestaMision& propuesta) {
    if (semilla_fija_) {
        generador_.seed(*semilla_fija_);
    } else {
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

    for (const auto& [segundo, aparicion] :
         planificar_oleadas(propuesta.limite_segundos, terreno, generador_)) {
        oleadas_.push_back(AparicionProgramada{segundo, aparicion});
    }
    return terreno;
}

std::vector<Aparicion> PlanificadorCampoAbierto::avanzar(float segundos) {
    if (segundos > 0.0F) {
        segundos_transcurridos_ += segundos;
    }
    std::vector<Aparicion> vencidas;
    while (siguiente_aparicion_ < oleadas_.size() &&
           oleadas_[siguiente_aparicion_].segundo <= segundos_transcurridos_) {
        vencidas.push_back(oleadas_[siguiente_aparicion_].aparicion);
        ++siguiente_aparicion_;
    }
    return vencidas;
}

} // namespace veterinaria