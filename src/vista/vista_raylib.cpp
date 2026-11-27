#include "vista_raylib.h"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace veterinaria {
namespace {

constexpr char titulo_ventana[] = "Pet Survivors";
constexpr char fuente_textos[] = "arte/fuentes/Handjet.ttf";
constexpr char fuente_encabezados[] = "arte/fuentes/PixelifySans.ttf";
constexpr char fondo_inicio[] = "arte/inicio/fondo.png";
constexpr char titulo_inicio[] = "arte/inicio/titulo.png";
constexpr char boton_jugar[] = "arte/inicio/boton_jugar.png";
constexpr char cursor_normal[] = "arte/inicio/cursor_normal.png";
constexpr char fondo_clinica[] = "arte/clinica/fondo.png";
constexpr char archivo_clic[] = "arte/audio/boton.wav";
constexpr char archivo_musica[] = "arte/audio/refugio.wav";
constexpr float volumen_clic = 0.50F;
constexpr float volumen_musica = 0.32F;
constexpr char pasto_mapa[] = "arte/mapa/pasto.png";
constexpr char tierra_mapa[] = "arte/mapa/tierra.png";
constexpr char transicion_mapa[] = "arte/mapa/transicion_pasto_tierra.png";
constexpr char arbol_mapa[] = "arte/mapa/obstaculos/arbol.png";
constexpr char roca_mapa[] = "arte/mapa/obstaculos/roca.png";
constexpr char tronco_mapa[] = "arte/mapa/obstaculos/tronco.png";
constexpr char hierba_mapa[] = "arte/mapa/decoracion/hierba_seca.png";
constexpr char escombros_mapa[] = "arte/mapa/decoracion/escombros.png";

struct RecursoSuministro {
    const char* id;
    const char* ruta;
};

constexpr std::array recursos_suministros{
    RecursoSuministro{"antiseptico", "arte/suministros/antiseptico.png"},
    RecursoSuministro{"suero", "arte/suministros/suero.png"},
    RecursoSuministro{"vendas", "arte/suministros/vendas.png"},
};

struct HojaSprites {
    const char* ruta;
    int ancho;
    int fotogramas;
    int alto = ancho;
};

struct HojasOrientadas {
    HojaSprites derecha;
    HojaSprites izquierda;
};

constexpr HojasOrientadas caminar_veterinario{
    {"arte/veterinario/caminar_derecha.png", 32, 4},
    {"arte/veterinario/caminar_izquierda.png", 32, 4},
};
constexpr HojasOrientadas caminar_perro{
    {"arte/enemigos/perro_asilvestrado/caminar_derecha.png", 24, 4},
    {"arte/enemigos/perro_asilvestrado/caminar_izquierda.png", 24, 4},
};
constexpr HojasOrientadas caminar_rata{
    {"arte/enemigos/rata_mutada/caminar_derecha.png", 24, 4},
    {"arte/enemigos/rata_mutada/caminar_izquierda.png", 24, 4},
};
constexpr HojasOrientadas caminar_saqueador{
    {"arte/enemigos/saqueador/caminar_derecha.png", 32, 4},
    {"arte/enemigos/saqueador/caminar_izquierda.png", 32, 4},
};
constexpr HojasOrientadas caminar_errante{
    {"arte/enemigos/errante_radiactivo/caminar_derecha.png", 32, 4},
    {"arte/enemigos/errante_radiactivo/caminar_izquierda.png", 32, 4},
};
constexpr HojasOrientadas caminar_vigia{
    {"arte/enemigos/vigia/caminar_derecha.png", 32, 4},
    {"arte/enemigos/vigia/caminar_izquierda.png", 32, 4},
};
constexpr HojasOrientadas lanzar_vigia{
    {"arte/enemigos/vigia/lanzar_derecha.png", 32, 4},
    {"arte/enemigos/vigia/lanzar_izquierda.png", 32, 4},
};
constexpr HojasOrientadas atacar_latigo{
    {"arte/armas/latigo/barrido_derecha.png", 112, 4, 64},
    {"arte/armas/latigo/barrido_izquierda.png", 112, 4, 64},
};
constexpr HojaSprites efecto_danio{"arte/efectos/danio_veterinario.png", 16, 3};
constexpr HojaSprites efecto_impacto{"arte/efectos/impacto_latigo.png", 16, 3};
constexpr HojaSprites efecto_recogida{"arte/efectos/recoger_suministro.png", 16, 3};
constexpr char rayo_vigia[] = "arte/enemigos/vigia/rayo.png";
constexpr char icono_latigo[] = "arte/armas/latigo/latigo.png";
constexpr char icono_vida[] = "arte/interfaz/icono_vida.png";
constexpr char icono_saldo[] = "arte/interfaz/icono_saldo.png";

constexpr std::array hojas_orientadas{
    caminar_veterinario, caminar_perro, caminar_rata, caminar_saqueador,
    caminar_errante,     caminar_vigia, lanzar_vigia, atacar_latigo,
};
constexpr std::array hojas_efectos{efecto_danio, efecto_impacto, efecto_recogida};
constexpr std::array iconos_combate{rayo_vigia, icono_latigo, icono_vida, icono_saldo};
constexpr float segundos_fotograma_caminar = 0.12F;
constexpr float segundos_fotograma_gesto = 0.065F;
constexpr float segundos_fotograma_efecto = 0.09F;
constexpr float segundos_fotograma_latigo = 0.095F;
constexpr float alcance_hoja_latigo = 96.0F;
constexpr Vector2 ancla_latigo{12.0F, 40.0F};
constexpr float altura_mano = 8.0F;
constexpr Color estela_vigia{164, 201, 82, 154};
constexpr Color nucleo_rayo{230, 247, 178, 235};

constexpr std::array texturas_fijas{
    fondo_inicio,   titulo_inicio, boton_jugar,     cursor_normal, fondo_clinica,
    pasto_mapa,     arbol_mapa,    roca_mapa,       tronco_mapa,   hierba_mapa,
    escombros_mapa, tierra_mapa,   transicion_mapa,
};

constexpr int ancho = 640;
constexpr int alto = 360;
constexpr int escala_ventana_inicial = 2;
constexpr int detalle_lienzo = 2;
constexpr int cuadros_por_segundo = 60;
constexpr int resolucion_fuente = 48;
constexpr float texto_boton = 18;
constexpr float texto_dialogo = 18;
constexpr float opacidad_pausa = 0.68f;
constexpr Vector2 punta_cursor{2, 2};
constexpr Vector2 tamano_cursor{24, 22};

constexpr Color carbon{22, 31, 38, 255};
constexpr Color petroleo{35, 60, 73, 255};
constexpr Color gris{125, 144, 149, 255};
constexpr Color crema{243, 233, 210, 255};
constexpr Color ladrillo{223, 125, 118, 255};
constexpr Color menta{135, 199, 161, 255};
constexpr Color suelo_base{37, 50, 44, 255};
constexpr Color color_pasto{88, 99, 70, 255};
constexpr Color color_tierra{105, 83, 64, 255};
constexpr Color copa_arbol{61, 83, 61, 255};
constexpr float celda_decoracion = 56;
constexpr float lado_suministro = 18;
constexpr Rectangle area_aviso_recogida{208, 309, 224, 31};
constexpr Rectangle area_guia_mision{474, 309, 154, 42};
constexpr Vector2 centro_flecha_mision{609, 330};

struct RecursoObstaculo {
    TipoObstaculo tipo;
    const char* ruta;
    Vector2 tamano;
};

constexpr std::array recursos_obstaculos{
    RecursoObstaculo{TipoObstaculo::arbol, arbol_mapa, {32, 48}},
    RecursoObstaculo{TipoObstaculo::roca, roca_mapa, {32, 24}},
    RecursoObstaculo{TipoObstaculo::tronco, tronco_mapa, {48, 24}},
};

// Coordenadas en el lienzo logico.
constexpr Rectangle lienzo_completo{0, 0, ancho, alto};
constexpr Rectangle area_titulo_inicio{29, 94, 256, 85};
constexpr Rectangle area_jugar{93, 198, 128, 43};
constexpr Rectangle area_nueva_partida{93, 248, 128, 31};

constexpr Rectangle area_saldo{508, 12, 120, 28};
constexpr Rectangle area_dueno{7, 111, 120, 160};
constexpr Rectangle area_mascota{127, 110, 84, 84};
constexpr Rectangle panel_dialogo{12, 267, 616, 81};
constexpr Rectangle area_linea_dialogo{24, 298, 410, 43};
constexpr Rectangle area_indicacion_clinica{24, 292, 410, 48};
constexpr Rectangle area_accion_clinica{444, 307, 170, 30};

struct OpcionClinica {
    Accion accion;
    const char* rotulo;
};

constexpr std::array opciones_clinica{
    OpcionClinica{Accion::examinar, "Examinar mascota"},
    OpcionClinica{Accion::avanzar_dialogo, "Continuar"},
    OpcionClinica{Accion::reintentar, "Reintentar"},
};

constexpr Rectangle panel_ficha{355, 48, 273, 211};
constexpr Rectangle marco_retrato_ficha{520, 82, 96, 96};
constexpr float margen_retrato_ficha = 4;
constexpr Rectangle retrato_ficha{
    marco_retrato_ficha.x + margen_retrato_ficha,
    marco_retrato_ficha.y + margen_retrato_ficha,
    marco_retrato_ficha.width - 2 * margen_retrato_ficha,
    marco_retrato_ficha.height - 2 * margen_retrato_ficha,
};
constexpr Rectangle area_nombre_ficha{367, 85, 145, 39};
constexpr Rectangle area_aceptar{367, 225, 115, 26};
constexpr Rectangle area_rechazar{492, 225, 123, 26};
constexpr Vector2 lista_ficha{367, 178};
constexpr float salto_fila_ficha = 15;
constexpr int filas_ficha = 3;
constexpr Rectangle panel_reloj{495, 12, 133, 64};
constexpr Rectangle marco_vida{506, 60, 110, 9};
constexpr float margen_barra_vida = 1;
constexpr float umbral_vida_baja = 0.25f;
constexpr float ancho_vida_vigia = 20.0F;
constexpr float altura_vida_vigia = 34.0F;
constexpr int filas_hud = 8;
constexpr float salto_fila_hud = 17;

constexpr Rectangle panel_pausa{192, 60, 256, 240};
constexpr Rectangle area_reanudar{210, 110, 220, 31};
constexpr Rectangle area_guardar{210, 149, 220, 31};
constexpr Rectangle area_reiniciar{210, 188, 220, 31};
constexpr Rectangle area_salir{210, 227, 220, 31};
constexpr Rectangle panel_confirmacion{157, 104, 326, 150};
constexpr Rectangle area_aviso_reinicio{174, 156, 290, 40};
constexpr Rectangle area_cancelar_reinicio{174, 205, 136, 31};
constexpr Rectangle area_confirmar_reinicio{322, 205, 144, 31};

class VistaRaylib final : public Vista {
  public:
    VistaRaylib();
    ~VistaRaylib() override;
    VistaRaylib(const VistaRaylib&) = delete;
    VistaRaylib& operator=(const VistaRaylib&) = delete;

    EntradaCuadro leer_entrada() override;
    void dibujar(const EstadoPantalla& estado) override;

  private:
    struct Boton {
        Rectangle area;
        Accion accion;
    };

    Rectangle area_de_ventana() const;
    Vector2 raton_en_lienzo() const;
    void iniciar_audio();
    void reproducir_clic() const;
    void cargar_textura(const std::string& ruta);
    Texture2D textura(const std::string& ruta) const;
    bool imagen(const std::string& ruta, Rectangle destino, Color tinte = WHITE) const;
    bool dibujar_fotograma(const HojaSprites& hoja, int indice, Vector2 centro,
                           bool reflejar = false, Vector2 escala = {1.0F, 1.0F}) const;
    bool dibujar_hoja_orientada(const HojasOrientadas& hojas, OrientacionHorizontal orientacion,
                                int indice, Vector2 centro, Vector2 escala = {1.0F, 1.0F}) const;
    void dibujar_efecto(const HojaSprites& hoja, float segundos, Vector2 centro) const;
    void preparar_retratos(const EstadoPantalla& estado);
    void texto(const std::string& contenido, Vector2 posicion, float tamano, Color color) const;
    void titulo(const char* contenido, Vector2 posicion, float tamano, Color color) const;
    void parrafo(const std::string& contenido, Rectangle area, float tamano, Color color) const;
    void boton(const std::string& rotulo, Rectangle area, Accion accion, bool imagen_jugar = false);
    void dibujar_inicio(const EstadoPantalla& estado);
    void dibujar_clinica(const EstadoPantalla& estado);
    void dibujar_ficha(const EstadoPantalla& estado);
    void dibujar_mision(const EstadoMision& estado, const EstadoPantalla& pantalla);
    void dibujar_suelo(const Terreno& terreno, Punto camara) const;
    void dibujar_decoracion(const Rectangulo& limites, Punto camara) const;
    void dibujar_suministro(const CantidadSuministro& suministro, Vector2 posicion) const;
    void dibujar_obstaculo(TipoObstaculo tipo, Vector2 anclaje) const;
    void dibujar_enemigo(const EstadoEnemigo& enemigo, Vector2 posicion, float segundos) const;
    void dibujar_vida_vigia(const EstadoEnemigo& enemigo, Vector2 posicion) const;
    void dibujar_veterinario(const EstadoCombate& combate, Vector2 posicion) const;
    void dibujar_proyectil(const EstadoProyectil& proyectil, Vector2 origen,
                           Vector2 posicion) const;
    void dibujar_hud_mision(const EstadoMision& estado);
    void dibujar_guia_mision(const EstadoMision& estado, Punto camara) const;
    void dibujar_vida(const EstadoCombate& combate) const;
    void dibujar_pausa(const EstadoPantalla& estado);

    RenderTexture2D lienzo_{};
    Font fuente_{};
    Font fuente_titulos_{};
    Sound clic_{};
    Music musica_{};
    std::unordered_map<std::string, Texture2D> texturas_;
    std::vector<Boton> botones_;
};

bool permite(const EstadoPantalla& estado, Accion accion) {
    const auto& acciones = estado.acciones_disponibles;
    return std::find(acciones.begin(), acciones.end(), accion) != acciones.end();
}

std::string nombre_suministro(const std::string& id) {
    if (id == "vendas") {
        return "Vendas";
    }

    if (id == "antiseptico") {
        return "Antiséptico";
    }

    if (id == "suero") {
        return "Suero";
    }
    return id;
}

int fotograma_caminar(const HojaSprites& hoja, EstadoMovimiento movimiento, float segundos) {
    if (!movimiento.en_movimiento) {
        return 0;
    }

    return static_cast<int>(segundos / segundos_fotograma_caminar) % hoja.fotogramas;
}

int fotograma_temporal(const HojaSprites& hoja, float segundos, float duracion) {
    if (segundos < 0.0F || segundos >= duracion * hoja.fotogramas) {
        return -1;
    }

    return static_cast<int>(segundos / duracion);
}

Vector2 centro_actor(Vector2 anclaje, const HojaSprites& hoja) {
    return {anclaje.x, anclaje.y - hoja.alto / 2.0F + 2.0F};
}

std::string reloj(float segundos) {
    const int total = static_cast<int>(std::ceil(std::max(0.0f, segundos)));
    std::ostringstream salida;
    salida << std::setfill('0') << std::setw(2) << total / 60 << ':' << std::setw(2) << total % 60;
    return salida.str();
}

const char* mensaje_de_fase(FasePantalla fase) {
    switch (fase) {
    case FasePantalla::por_examinar:
        return "Examina a la mascota para saber qué necesita.";
    case FasePantalla::resultado_examen:
        return "Revisa la ficha antes de decidir.";
    case FasePantalla::reintento:
        return "Puedes volver a buscar los suministros.";
    case FasePantalla::sin_casos:
        return "No quedan casos.";
    default:
        return "";
    }
}

void panel(Rectangle area, Color relleno = carbon) {
    DrawRectangleRec(area, relleno);
    DrawRectangleLinesEx(area, 1, petroleo);
}

Font cargar_fuente(const char* ruta) {
    const auto predeterminada = GetFontDefault();

    if (!FileExists(ruta)) {
        return predeterminada;
    }
    std::vector<int> caracteres;

    for (int codigo = 32; codigo <= 126; ++codigo) {
        caracteres.push_back(codigo);
    }

    for (int codigo = 160; codigo <= 255; ++codigo) {
        caracteres.push_back(codigo);
    }
    caracteres.push_back(0x2026);
    const int cantidad_caracteres = static_cast<int>(caracteres.size());
    const Font fuente = LoadFontEx(ruta, resolucion_fuente, caracteres.data(), cantidad_caracteres);

    if (fuente.texture.id == 0 || fuente.texture.id == predeterminada.texture.id) {
        return predeterminada;
    }
    SetTextureFilter(fuente.texture, TEXTURE_FILTER_BILINEAR);
    return fuente;
}

} // namespace

Vista::~Vista() = default;

VistaRaylib::VistaRaylib() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(ancho * escala_ventana_inicial, alto * escala_ventana_inicial, titulo_ventana);

    if (!IsWindowReady()) {
        throw std::runtime_error("No se pudo abrir la ventana gráfica.");
    }
    SetWindowMinSize(ancho, alto);
    SetTargetFPS(cuadros_por_segundo);
    SetExitKey(KEY_NULL);
    lienzo_ = LoadRenderTexture(ancho * detalle_lienzo, alto * detalle_lienzo);

    if (lienzo_.id == 0) {
        CloseWindow();
        throw std::runtime_error("No se pudo crear el lienzo gráfico.");
    }
    SetTextureFilter(lienzo_.texture, TEXTURE_FILTER_POINT);
    fuente_ = cargar_fuente(fuente_textos);
    fuente_titulos_ = cargar_fuente(fuente_encabezados);

    for (const char* ruta : texturas_fijas) {
        cargar_textura(ruta);
    }

    for (const auto& recurso : recursos_suministros) {
        cargar_textura(recurso.ruta);
    }

    for (const auto& hojas : hojas_orientadas) {
        cargar_textura(hojas.derecha.ruta);
        cargar_textura(hojas.izquierda.ruta);
    }

    for (const auto& hoja : hojas_efectos) {
        cargar_textura(hoja.ruta);
    }

    for (const char* ruta : iconos_combate) {
        cargar_textura(ruta);
    }

    if (textura(cursor_normal).id != 0) {
        HideCursor();
    }

    iniciar_audio();
}

VistaRaylib::~VistaRaylib() {
    if (IsAudioDeviceReady()) {
        if (IsSoundValid(clic_)) {
            UnloadSound(clic_);
        }

        if (IsMusicValid(musica_)) {
            UnloadMusicStream(musica_);
        }

        CloseAudioDevice();
    }

    for (const auto& par : texturas_) {
        if (par.second.id != 0) {
            UnloadTexture(par.second);
        }
    }
    const auto predeterminada = GetFontDefault();

    for (const Font fuente : {fuente_, fuente_titulos_}) {
        if (fuente.texture.id != predeterminada.texture.id) {
            UnloadFont(fuente);
        }
    }
    UnloadRenderTexture(lienzo_);
    CloseWindow();
}

void VistaRaylib::iniciar_audio() {
    InitAudioDevice();

    if (!IsAudioDeviceReady()) {
        return;
    }

    clic_ = LoadSound(archivo_clic);

    if (IsSoundValid(clic_)) {
        SetSoundVolume(clic_, volumen_clic);
    }

    musica_ = LoadMusicStream(archivo_musica);

    if (IsMusicValid(musica_)) {
        musica_.looping = true;
        SetMusicVolume(musica_, volumen_musica);
        PlayMusicStream(musica_);
    }
}

void VistaRaylib::reproducir_clic() const {
    if (IsSoundValid(clic_)) {
        PlaySound(clic_);
    }
}

Rectangle VistaRaylib::area_de_ventana() const {
    const float escala_horizontal = static_cast<float>(GetScreenWidth()) / ancho;
    const float escala_vertical = static_cast<float>(GetScreenHeight()) / alto;
    const float escala = std::min(escala_horizontal, escala_vertical);
    const float ancho_visible = ancho * escala;
    const float alto_visible = alto * escala;
    return {
        std::floor((GetScreenWidth() - ancho_visible) / 2),
        std::floor((GetScreenHeight() - alto_visible) / 2),
        ancho_visible,
        alto_visible,
    };
}

Vector2 VistaRaylib::raton_en_lienzo() const {
    const auto area = area_de_ventana();

    if (area.width <= 0 || area.height <= 0) {
        return {-1, -1};
    }
    const auto raton = GetMousePosition();
    return {
        (raton.x - area.x) * ancho / area.width,
        (raton.y - area.y) * alto / area.height,
    };
}

EntradaCuadro VistaRaylib::leer_entrada() {
    EntradaCuadro entrada{};
    entrada.salir = WindowShouldClose();
    entrada.direccion = {
        static_cast<float>(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)),
        static_cast<float>(IsKeyDown(KEY_S) - IsKeyDown(KEY_W)),
    };

    const bool hay_boton_principal =
        !botones_.empty() &&
        (botones_.size() == 1 || botones_.front().accion == Accion::iniciar_partida);

    if (IsKeyPressed(KEY_ESCAPE)) {
        entrada.accion = Accion::alternar_menu;
    }

    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const auto raton = raton_en_lienzo();

        for (const auto& actual : botones_) {
            if (CheckCollisionPointRec(raton, actual.area)) {
                entrada.accion = actual.accion;
                reproducir_clic();
                break;
            }
        }
    }

    else if (IsKeyPressed(KEY_ENTER) && hay_boton_principal) {
        entrada.accion = botones_.front().accion;
        reproducir_clic();
    }
    return entrada;
}

void VistaRaylib::cargar_textura(const std::string& ruta) {
    if (ruta.empty() || texturas_.find(ruta) != texturas_.end()) {
        return;
    }

    Texture2D cargada{};

    if (FileExists(ruta.c_str())) {
        cargada = LoadTexture(ruta.c_str());

        if (cargada.id != 0) {
            SetTextureFilter(cargada, TEXTURE_FILTER_POINT);
        }
    }

    texturas_.emplace(ruta, cargada);
}

Texture2D VistaRaylib::textura(const std::string& ruta) const {
    const auto existente = texturas_.find(ruta);
    return existente == texturas_.end() ? Texture2D{} : existente->second;
}

void VistaRaylib::preparar_retratos(const EstadoPantalla& estado) {
    if (estado.pantalla != Pantalla::clinica || !estado.ficha) {
        return;
    }

    cargar_textura(estado.ficha->dueno.sprite);
    cargar_textura(estado.ficha->mascota.retrato);

    if (estado.dialogo) {
        cargar_textura(estado.dialogo->sprite_dueno);
        cargar_textura(estado.dialogo->retrato_mascota);
    }
}

bool VistaRaylib::imagen(const std::string& ruta, Rectangle destino, Color tinte) const {
    const auto recurso = textura(ruta);

    if (recurso.id == 0) {
        return false;
    }
    const Rectangle origen{
        0,
        0,
        static_cast<float>(recurso.width),
        static_cast<float>(recurso.height),
    };
    DrawTexturePro(recurso, origen, destino, {0, 0}, 0, tinte);
    return true;
}

bool VistaRaylib::dibujar_fotograma(const HojaSprites& hoja, int indice, Vector2 centro,
                                    bool reflejar, Vector2 escala) const {
    if (indice < 0 || indice >= hoja.fotogramas) {
        return false;
    }

    const Texture2D recurso = textura(hoja.ruta);

    if (recurso.id == 0 || recurso.width != hoja.ancho * hoja.fotogramas ||
        recurso.height != hoja.alto) {
        return false;
    }

    const float ancho_celda = static_cast<float>(hoja.ancho);
    const float alto_celda = static_cast<float>(hoja.alto);
    const Rectangle origen{indice * ancho_celda, 0.0F, reflejar ? -ancho_celda : ancho_celda,
                           alto_celda};
    const float ancho = ancho_celda * escala.x;
    const float alto = alto_celda * escala.y;
    const Rectangle destino{centro.x - ancho / 2.0F, centro.y - alto / 2.0F, ancho, alto};
    DrawTexturePro(recurso, origen, destino, {0.0F, 0.0F}, 0.0F, WHITE);
    return true;
}

bool VistaRaylib::dibujar_hoja_orientada(const HojasOrientadas& hojas,
                                         OrientacionHorizontal orientacion, int indice,
                                         Vector2 centro, Vector2 escala) const {
    const bool izquierda = orientacion == OrientacionHorizontal::izquierda;
    const auto& preferida = izquierda ? hojas.izquierda : hojas.derecha;

    if (dibujar_fotograma(preferida, indice, centro, false, escala)) {
        return true;
    }

    const auto& opuesta = izquierda ? hojas.derecha : hojas.izquierda;
    return dibujar_fotograma(opuesta, indice, centro, true, escala);
}

void VistaRaylib::dibujar_efecto(const HojaSprites& hoja, float segundos, Vector2 centro) const {
    const int indice = fotograma_temporal(hoja, segundos, segundos_fotograma_efecto);

    if (indice >= 0) {
        dibujar_fotograma(hoja, indice, centro);
    }
}

void VistaRaylib::texto(const std::string& contenido, Vector2 posicion, float tamano,
                        Color color) const {
    DrawTextEx(fuente_, contenido.c_str(), posicion, tamano, 1.0f, color);
}

void VistaRaylib::titulo(const char* contenido, Vector2 posicion, float tamano, Color color) const {
    DrawTextEx(fuente_titulos_, contenido, posicion, tamano, 1.0f, color);
}

void VistaRaylib::parrafo(const std::string& contenido, Rectangle area, float tamano,
                          Color color) const {
    std::istringstream palabras(contenido);
    std::string palabra;
    std::string linea;
    float y = area.y;

    while (palabras >> palabra) {
        const std::string candidata = linea.empty() ? palabra : linea + ' ' + palabra;

        if (!linea.empty() && MeasureTextEx(fuente_, candidata.c_str(), tamano, 1).x > area.width) {
            texto(linea, {area.x, y}, tamano, color);
            y += tamano + 2;
            linea = palabra;
        }

        else {
            linea = candidata;
        }

        if (y + tamano > area.y + area.height) {
            return;
        }
    }
    texto(linea, {area.x, y}, tamano, color);
}

void VistaRaylib::boton(const std::string& rotulo, Rectangle area, Accion accion,
                        bool imagen_jugar) {
    botones_.push_back({area, accion});
    const bool encima = CheckCollisionPointRec(raton_en_lienzo(), area);

    if (imagen_jugar && imagen(boton_jugar, area, encima ? crema : WHITE)) {
        if (encima) {
            DrawRectangleLinesEx(area, 1, crema);
        }
        return;
    }
    panel(area, encima ? petroleo : carbon);
    DrawRectangleLinesEx(area, 1, encima ? crema : gris);
    const auto medida = MeasureTextEx(fuente_, rotulo.c_str(), texto_boton, 1);
    const Vector2 centro{
        std::floor(area.x + (area.width - medida.x) / 2),
        std::floor(area.y + (area.height - medida.y) / 2),
    };
    const Color color_rotulo = accion == Accion::rechazar ? ladrillo : crema;
    texto(rotulo, centro, texto_boton, color_rotulo);
}

void VistaRaylib::dibujar(const EstadoPantalla& estado) {
    if (IsMusicValid(musica_)) {
        UpdateMusicStream(musica_);
    }

    preparar_retratos(estado);
    botones_.clear();
    BeginTextureMode(lienzo_);
    ClearBackground(carbon);
    BeginMode2D({{0, 0}, {0, 0}, 0, detalle_lienzo});

    switch (estado.pantalla) {
    case Pantalla::inicio:
        dibujar_inicio(estado);
        break;
    case Pantalla::clinica:
        dibujar_clinica(estado);
        break;
    case Pantalla::mision:
        if (estado.mision) {
            dibujar_mision(*estado.mision, estado);

            if (estado.aviso_recogida) {
                const auto& recogido = *estado.aviso_recogida;
                panel(area_aviso_recogida);
                texto("+" + std::to_string(recogido.cantidad) + " " +
                          nombre_suministro(recogido.id),
                      {area_aviso_recogida.x + 12, area_aviso_recogida.y + 6}, 17, menta);
            }
        }
        break;
    }

    if (estado.opacidad_fundido > 0.0f) {
        DrawRectangle(0, 0, ancho, alto,
                      Fade(BLACK, std::clamp(estado.opacidad_fundido, 0.0f, 1.0f)));
        botones_.clear();
    }

    if (estado.menu_abierto) {
        botones_.clear();
        dibujar_pausa(estado);
    }
    const auto raton = raton_en_lienzo();
    const Rectangle area_cursor{
        std::floor(raton.x) - punta_cursor.x,
        std::floor(raton.y) - punta_cursor.y,
        tamano_cursor.x,
        tamano_cursor.y,
    };
    imagen(cursor_normal, area_cursor);
    EndMode2D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(lienzo_.texture, {0, 0, ancho * detalle_lienzo, -alto * detalle_lienzo},
                   area_de_ventana(), {0, 0}, 0, WHITE);
    EndDrawing();
}

void VistaRaylib::dibujar_inicio(const EstadoPantalla& estado) {
    imagen(fondo_inicio, lienzo_completo);

    if (!imagen(titulo_inicio, area_titulo_inicio)) {
        titulo("PET SURVIVORS", {44, 120}, 24, crema);
    }

    if (permite(estado, Accion::iniciar_partida)) {
        boton("Jugar", area_jugar, Accion::iniciar_partida, true);
    }

    if (permite(estado, Accion::nueva_partida)) {
        boton("Nueva partida", area_nueva_partida, Accion::nueva_partida);
    }
}

void VistaRaylib::dibujar_clinica(const EstadoPantalla& estado) {
    imagen(fondo_clinica, lienzo_completo);
    panel(area_saldo);
    const bool hay_icono = imagen(icono_saldo, {518, 20, 12, 12});
    texto("Saldo: " + std::to_string(estado.saldo), {hay_icono ? 535.0F : 518.0F, 17}, 17, crema);
    const auto* ficha = estado.ficha;

    if (ficha) {
        const bool reaccion_dueno =
            estado.dialogo && imagen(estado.dialogo->sprite_dueno, area_dueno);
        const bool reaccion_mascota =
            estado.dialogo && imagen(estado.dialogo->retrato_mascota, area_mascota);

        if (!reaccion_dueno && !imagen(ficha->dueno.sprite, area_dueno)) {
            texto(ficha->dueno.nombre, {22, 238}, 14, crema);
        }

        if (!reaccion_mascota && !imagen(ficha->mascota.retrato, area_mascota)) {
            texto(ficha->mascota.nombre, {139, 166}, 14, crema);
        }
    }

    if (estado.fase == FasePantalla::resultado_examen && ficha) {
        dibujar_ficha(estado);
    }

    panel(panel_dialogo);

    if (estado.dialogo) {
        const std::string hablante = estado.dialogo->hablante == Hablante::dueno && ficha
                                         ? ficha->dueno.nombre
                                         : "Veterinario";
        texto(hablante, {24, 275}, texto_dialogo, menta);
        parrafo(estado.dialogo->texto, area_linea_dialogo, texto_dialogo, crema);
    }

    else {
        parrafo(mensaje_de_fase(estado.fase), area_indicacion_clinica, texto_dialogo, crema);
    }

    if (estado.motivo_fallo) {
        texto(*estado.motivo_fallo == MotivoFalloMision::tiempo_agotado ? "Tiempo agotado"
                                                                        : "El veterinario cayó",
              {439, 275}, 15, ladrillo);
    }

    for (const auto& opcion : opciones_clinica) {
        if (permite(estado, opcion.accion)) {
            boton(opcion.rotulo, area_accion_clinica, opcion.accion);
            break;
        }
    }
}

void VistaRaylib::dibujar_ficha(const EstadoPantalla& estado) {
    const auto& ficha = *estado.ficha;
    panel(panel_ficha);
    titulo("Ficha de examen", {367, 58}, 21, menta);
    panel(marco_retrato_ficha, petroleo);
    imagen(ficha.mascota.retrato, retrato_ficha);
    const std::string nombre = ficha.mascota.nombre + " / " + ficha.mascota.especie;
    parrafo(nombre, area_nombre_ficha, texto_dialogo, crema);
    texto("Edad: " + std::to_string(ficha.mascota.edad_meses) + " meses", {367, 123}, 16, crema);

    if (estado.edad_humana_mascota) {
        std::ostringstream equivalente;
        equivalente << std::fixed << std::setprecision(1) << *estado.edad_humana_mascota;
        texto(equivalente.str() + " años humanos", {367, 141}, 15, crema);
    }
    titulo("Suministros", {367, 157}, 17, menta);
    float y = lista_ficha.y;
    int filas_mostradas = 0;

    for (const auto& suministro : ficha.suministros) {
        if (filas_mostradas == filas_ficha) {
            texto("…", {591, 204}, 14, gris);
            break;
        }
        texto(nombre_suministro(suministro.id) + " x " + std::to_string(suministro.cantidad),
              {lista_ficha.x, y}, 16, crema);
        y += salto_fila_ficha;
        ++filas_mostradas;
    }

    if (permite(estado, Accion::aceptar)) {
        boton("Aceptar", area_aceptar, Accion::aceptar);
    }

    if (permite(estado, Accion::rechazar)) {
        boton("Rechazar", area_rechazar, Accion::rechazar);
    }
}

void VistaRaylib::dibujar_mision(const EstadoMision& estado, const EstadoPantalla& pantalla) {
    const auto& combate = estado.combate;
    const auto& limites = estado.terreno.limites;
    const float camara_x_maxima = limites.origen.x + std::max(0.0f, limites.ancho - ancho);
    const float camara_y_maxima = limites.origen.y + std::max(0.0f, limites.alto - alto);
    const float camara_x =
        std::clamp(combate.jugador.x - ancho / 2.0F, limites.origen.x, camara_x_maxima);
    const float camara_y =
        std::clamp(combate.jugador.y - alto / 2.0F, limites.origen.y, camara_y_maxima);
    const auto visible = [camara_x, camara_y](Punto punto) -> Vector2 {
        return {std::floor(punto.x - camara_x), std::floor(punto.y - camara_y)};
    };
    dibujar_suelo(estado.terreno, {camara_x, camara_y});
    dibujar_decoracion(limites, {camara_x, camara_y});

    for (const auto& obstaculo : estado.terreno.obstaculos) {
        dibujar_obstaculo(obstaculo.tipo, visible(obstaculo.anclaje));
    }

    for (const auto& suministro : estado.terreno.suministros) {
        dibujar_suministro(suministro.contenido, visible(suministro.posicion));
    }

    for (const auto& enemigo : combate.enemigos) {
        const auto posicion = visible(enemigo.posicion);
        dibujar_enemigo(enemigo, posicion, combate.segundos_combate);

        if (enemigo.tipo == TipoEnemigo::superviviente_a_distancia &&
            enemigo.vida < enemigo.vida_maxima) {
            dibujar_vida_vigia(enemigo, posicion);
        }
    }

    for (const auto& proyectil : combate.proyectiles) {
        dibujar_proyectil(proyectil, visible(proyectil.origen), visible(proyectil.posicion));
    }

    const auto jugador = visible(combate.jugador);
    dibujar_veterinario(combate, jugador);

    if (combate.ultimo_golpe && combate.ultimo_golpe->impacto) {
        const float segundos = combate.segundos_combate - combate.ultimo_golpe->gesto.instante;
        auto impacto = visible(*combate.ultimo_golpe->impacto);
        impacto.y -= 8.0F;
        dibujar_efecto(efecto_impacto, segundos, impacto);
    }

    if (pantalla.aviso_recogida) {
        auto posicion = visible(pantalla.posicion_recogida);
        posicion.y -= 15.0F;
        dibujar_efecto(efecto_recogida, pantalla.segundos_desde_recogida, posicion);
    }

    dibujar_hud_mision(estado);
    dibujar_guia_mision(estado, {camara_x, camara_y});
}

void VistaRaylib::dibujar_enemigo(const EstadoEnemigo& enemigo, Vector2 posicion,
                                  float segundos) const {
    const HojasOrientadas* principal = &caminar_vigia;
    const HojasOrientadas* variante = nullptr;

    switch (enemigo.tipo) {
    case TipoEnemigo::animal_de_contacto:
        principal = &caminar_perro;
        variante = &caminar_rata;
        break;
    case TipoEnemigo::superviviente_de_contacto:
        principal = &caminar_saqueador;
        variante = &caminar_errante;
        break;
    case TipoEnemigo::superviviente_a_distancia:
        if (enemigo.disparo) {
            const float edad = segundos - enemigo.disparo->instante;
            const int indice =
                fotograma_temporal(lanzar_vigia.derecha, edad, segundos_fotograma_gesto);

            if (dibujar_hoja_orientada(lanzar_vigia, enemigo.disparo->orientacion, indice,
                                       centro_actor(posicion, lanzar_vigia.derecha))) {
                return;
            }
        }
        break;
    }

    const int indice = fotograma_caminar(principal->derecha, enemigo.movimiento, segundos);
    const auto orientacion = enemigo.movimiento.orientacion;

    if (enemigo.variante_visual && variante &&
        dibujar_hoja_orientada(*variante, orientacion, indice,
                               centro_actor(posicion, variante->derecha))) {
        return;
    }

    if (dibujar_hoja_orientada(*principal, orientacion, indice,
                               centro_actor(posicion, principal->derecha))) {
        return;
    }

    if (enemigo.tipo == TipoEnemigo::animal_de_contacto) {
        DrawEllipse(static_cast<int>(posicion.x), static_cast<int>(posicion.y), 9, 5, ladrillo);
        return;
    }

    const Color color = enemigo.tipo == TipoEnemigo::superviviente_a_distancia ? ORANGE : ladrillo;
    DrawRectangleV({posicion.x - 6, posicion.y - 13}, {12, 16}, color);
}

void VistaRaylib::dibujar_vida_vigia(const EstadoEnemigo& enemigo, Vector2 posicion) const {
    const float proporcion = static_cast<float>(enemigo.vida) / enemigo.vida_maxima;
    const Rectangle fondo{posicion.x - ancho_vida_vigia / 2.0F, posicion.y - altura_vida_vigia,
                          ancho_vida_vigia, 3.0F};
    const Rectangle relleno{fondo.x + 1.0F, fondo.y + 1.0F,
                            std::round((fondo.width - 2.0F) * proporcion), 1.0F};
    DrawRectangleRec(fondo, carbon);
    DrawRectangleRec(relleno, ladrillo);
}

void VistaRaylib::dibujar_veterinario(const EstadoCombate& combate, Vector2 posicion) const {
    const auto movimiento = combate.movimiento_jugador;
    const int indice =
        fotograma_caminar(caminar_veterinario.derecha, movimiento, combate.segundos_combate);

    if (!dibujar_hoja_orientada(caminar_veterinario, movimiento.orientacion, indice,
                                centro_actor(posicion, caminar_veterinario.derecha))) {
        DrawRectangleV({posicion.x - 7, posicion.y - 13}, {14, 16}, crema);
        const bool izquierda = movimiento.orientacion == OrientacionHorizontal::izquierda;
        DrawRectangleV({posicion.x + (izquierda ? -9 : 4), posicion.y - 8}, {5, 6}, petroleo);
    }

    if (combate.ultimo_golpe) {
        const auto& golpe = *combate.ultimo_golpe;
        const auto& gesto = golpe.gesto;
        const int ataque =
            fotograma_temporal(atacar_latigo.derecha, combate.segundos_combate - gesto.instante,
                               segundos_fotograma_latigo);
        const auto& hoja = atacar_latigo.derecha;
        const float escala = golpe.alcance / alcance_hoja_latigo;
        const float sentido = gesto.orientacion == OrientacionHorizontal::izquierda ? -1.0F : 1.0F;
        const Vector2 centro{posicion.x + sentido * escala * (hoja.ancho / 2.0F - ancla_latigo.x),
                             posicion.y - altura_mano + hoja.alto / 2.0F - ancla_latigo.y};
        dibujar_hoja_orientada(atacar_latigo, gesto.orientacion, ataque, centro, {escala, 1.0F});
    }

    if (combate.instante_danio) {
        dibujar_efecto(efecto_danio, combate.segundos_combate - *combate.instante_danio,
                       {posicion.x, posicion.y - 12.0F});
    }
}

void VistaRaylib::dibujar_proyectil(const EstadoProyectil& proyectil, Vector2 origen_estela,
                                    Vector2 posicion) const {
    DrawLineEx(origen_estela, posicion, 3.0F, estela_vigia);
    DrawLineEx(origen_estela, posicion, 1.0F, nucleo_rayo);
    const auto recurso = textura(rayo_vigia);

    if (recurso.id == 0) {
        DrawCircleV(posicion, 2, crema);
        return;
    }

    const float angulo = std::atan2(proyectil.velocidad.y, proyectil.velocidad.x) * RAD2DEG;
    const Rectangle origen{0, 0, static_cast<float>(recurso.width),
                           static_cast<float>(recurso.height)};
    DrawTexturePro(recurso, origen, {posicion.x, posicion.y, 16, 8}, {8, 4}, angulo, WHITE);
}

void dibujar_pieza_suelo(Texture2D imagen, Rectangle area, PiezaSuelo pieza) {
    const int giros = pieza.tipo == TipoPiezaSuelo::transicion ? pieza.giro_cuartos % 4 : 0;
    const float rotacion = giros * 90.0F;
    const Vector2 centro{area.x + area.width / 2, area.y + area.height / 2};
    const float ancho_pieza = giros % 2 != 0 ? area.height : area.width;
    const float alto_pieza = giros % 2 != 0 ? area.width : area.height;
    const Rectangle destino{centro.x, centro.y, ancho_pieza, alto_pieza};
    const Vector2 origen{ancho_pieza / 2, alto_pieza / 2};

    if (imagen.id != 0) {
        const Rectangle fuente{0, 0, static_cast<float>(imagen.width),
                               static_cast<float>(imagen.height)};
        DrawTexturePro(imagen, fuente, destino, origen, rotacion, WHITE);
        return;
    }

    DrawRectangleRec(area, pieza.tipo == TipoPiezaSuelo::tierra ? color_tierra : color_pasto);

    if (pieza.tipo == TipoPiezaSuelo::transicion) {
        DrawRectanglePro({centro.x, centro.y, ancho_pieza, alto_pieza / 2}, {origen.x, 0}, rotacion,
                         color_tierra);
    }
}

void VistaRaylib::dibujar_suelo(const Terreno& terreno, Punto camara) const {
    DrawRectangleRec(lienzo_completo, suelo_base);
    const auto& limites = terreno.limites;
    const auto& suelo = terreno.suelo;
    const float lado = suelo.tamano_pieza > 0 ? suelo.tamano_pieza : SueloMapa{}.tamano_pieza;
    const float x_camara = camara.x - limites.origen.x;
    const float y_camara = camara.y - limites.origen.y;
    const int columna_inicial = static_cast<int>(std::max(0.0F, x_camara) / lado);
    const int fila_inicial = static_cast<int>(std::max(0.0F, y_camara) / lado);
    const int columna_final =
        static_cast<int>(std::ceil(std::min(limites.ancho, x_camara + ancho) / lado));
    const int fila_final =
        static_cast<int>(std::ceil(std::min(limites.alto, y_camara + alto) / lado));
    const auto pasto = textura(pasto_mapa);
    const auto tierra = textura(tierra_mapa);
    const auto transicion = textura(transicion_mapa);

    for (int fila = fila_inicial; fila < fila_final; ++fila) {
        for (int columna = columna_inicial; columna < columna_final; ++columna) {
            const float x = columna * lado;
            const float y = fila * lado;
            const Rectangle area{std::floor(x - x_camara), std::floor(y - y_camara),
                                 std::min(lado, limites.ancho - x),
                                 std::min(lado, limites.alto - y)};
            const auto pieza = suelo.pieza(columna, fila);
            Texture2D imagen = pasto;

            if (pieza.tipo == TipoPiezaSuelo::tierra) {
                imagen = tierra;
            }

            else if (pieza.tipo == TipoPiezaSuelo::transicion) {
                imagen = transicion;
            }

            dibujar_pieza_suelo(imagen, area, pieza);
        }
    }
}

void VistaRaylib::dibujar_decoracion(const Rectangulo& limites, Punto camara) const {
    const int primera_columna = static_cast<int>((camara.x - limites.origen.x) / celda_decoracion);
    const int primera_fila = static_cast<int>((camara.y - limites.origen.y) / celda_decoracion);

    for (int fila = primera_fila; fila <= primera_fila + alto / celda_decoracion + 1; ++fila) {
        for (int columna = primera_columna;
             columna <= primera_columna + ancho / celda_decoracion + 1; ++columna) {
            const unsigned patron = static_cast<unsigned>(fila) * 73856093U ^
                                    static_cast<unsigned>(columna) * 19349663U;
            const float x = limites.origen.x + columna * celda_decoracion + patron % 23;
            const float y = limites.origen.y + fila * celda_decoracion + (patron / 23) % 27;

            if (x + 24 <= limites.origen.x + limites.ancho &&
                y + 16 <= limites.origen.y + limites.alto) {
                if (patron % 5 == 0) {
                    imagen(escombros_mapa, {x - camara.x, y - camara.y, 24, 16});
                }

                else if (patron % 5 <= 2) {
                    imagen(hierba_mapa, {x - camara.x, y - camara.y, 16, 16});
                }
            }
        }
    }
}

void VistaRaylib::dibujar_suministro(const CantidadSuministro& suministro, Vector2 posicion) const {
    const auto recurso = std::find_if(recursos_suministros.begin(), recursos_suministros.end(),
                                      [&suministro](const RecursoSuministro& candidato) {
                                          return candidato.id == suministro.id;
                                      });
    const Rectangle area{posicion.x - lado_suministro / 2, posicion.y - lado_suministro / 2,
                         lado_suministro, lado_suministro};

    if (recurso == recursos_suministros.end() || !imagen(recurso->ruta, area)) {
        DrawRectangleRec(area, menta);
        DrawLineV({posicion.x - 4, posicion.y}, {posicion.x + 4, posicion.y}, carbon);
        DrawLineV({posicion.x, posicion.y - 4}, {posicion.x, posicion.y + 4}, carbon);
    }

    texto(nombre_suministro(suministro.id), {posicion.x - 14, posicion.y + 10}, 11, crema);
}

void VistaRaylib::dibujar_obstaculo(TipoObstaculo tipo, Vector2 anclaje) const {
    const auto corresponde = [tipo](const RecursoObstaculo& recurso) {
        return recurso.tipo == tipo;
    };
    const auto recurso =
        std::find_if(recursos_obstaculos.begin(), recursos_obstaculos.end(), corresponde);

    if (recurso != recursos_obstaculos.end()) {
        const Rectangle destino{
            anclaje.x - recurso->tamano.x / 2,
            anclaje.y - recurso->tamano.y,
            recurso->tamano.x,
            recurso->tamano.y,
        };

        if (imagen(recurso->ruta, destino)) {
            return;
        }
    }

    switch (tipo) {
    case TipoObstaculo::arbol:
        DrawRectangleV({anclaje.x - 5, anclaje.y - 17}, {10, 21}, BROWN);
        DrawRectangleV({anclaje.x - 18, anclaje.y - 38}, {36, 24}, copa_arbol);
        break;
    case TipoObstaculo::roca:
    case TipoObstaculo::tronco:
        DrawRectangleV({anclaje.x - 15, anclaje.y - 8}, {30, 16},
                       tipo == TipoObstaculo::roca ? gris : BROWN);
        break;
    }
}

void VistaRaylib::dibujar_hud_mision(const EstadoMision& estado) {
    const auto total_filas = std::min<std::size_t>(estado.requeridos.size(), filas_hud + 1);
    panel({12, 12, 185, 38 + salto_fila_hud * total_filas});
    titulo("Suministros", {22, 19}, 16, menta);
    float y = 39;
    std::size_t mostrados = 0;

    for (const auto& requerido : estado.requeridos) {
        if (mostrados++ == filas_hud) {
            texto("…", {22, y}, 12, crema);
            break;
        }
        const auto recogido =
            std::find_if(estado.recogidos.begin(), estado.recogidos.end(),
                         [&](const auto& cantidad) { return cantidad.id == requerido.id; });
        const int cantidad = recogido == estado.recogidos.end() ? 0 : recogido->cantidad;
        texto(nombre_suministro(requerido.id) + "  " + std::to_string(cantidad) + "/" +
                  std::to_string(requerido.cantidad),
              {22, y}, 13, crema);
        y += salto_fila_hud;
    }
    panel(panel_reloj);
    texto(reloj(estado.segundos_restantes), {506, 18}, 19, crema);
    imagen(icono_latigo, {598, 20, 16, 16});
    dibujar_vida(estado.combate);
}

void VistaRaylib::dibujar_guia_mision(const EstadoMision& estado, Punto camara) const {
    const auto& jugador = estado.combate.jugador;
    const auto& suministros = estado.terreno.suministros;
    const auto distancia = [&jugador](const SuministroEnMapa& suministro) {
        return std::hypot(suministro.posicion.x - jugador.x, suministro.posicion.y - jugador.y);
    };
    const auto cercano = std::min_element(
        suministros.begin(), suministros.end(),
        [&distancia](const auto& a, const auto& b) { return distancia(a) < distancia(b); });

    if (cercano == suministros.end()) {
        return;
    }

    const Vector2 relativo{cercano->posicion.x - camara.x, cercano->posicion.y - camara.y};
    const float magnitud = distancia(*cercano);

    if (magnitud <= 0.0F || CheckCollisionPointRec(relativo, lienzo_completo)) {
        return;
    }

    const Vector2 direccion{(cercano->posicion.x - jugador.x) / magnitud,
                            (cercano->posicion.y - jugador.y) / magnitud};
    const Vector2 punta{centro_flecha_mision.x + direccion.x * 9,
                        centro_flecha_mision.y + direccion.y * 9};
    const Vector2 cola{centro_flecha_mision.x - direccion.x * 6,
                       centro_flecha_mision.y - direccion.y * 6};
    const Vector2 ala_a{cola.x - direccion.y * 5, cola.y + direccion.x * 5};
    const Vector2 ala_b{cola.x + direccion.y * 5, cola.y - direccion.x * 5};
    panel(area_guia_mision);
    texto("Más cercano", {484, 314}, 13, gris);
    texto(nombre_suministro(cercano->contenido.id), {484, 330}, 15, crema);
    DrawTriangle(punta, ala_b, ala_a, menta);
}

void VistaRaylib::dibujar_vida(const EstadoCombate& combate) const {
    const int vida_maxima = std::max(0, combate.vida_maxima);
    const int vida = std::clamp(combate.vida, 0, vida_maxima);
    const std::string rotulo = "Vida: " + std::to_string(vida) + "/" + std::to_string(vida_maxima);
    const bool hay_icono = imagen(icono_vida, {506, 43, 12, 12});
    texto(rotulo, {hay_icono ? 523.0F : 506.0F, 42}, 13, crema);
    panel(marco_vida, petroleo);

    if (vida == 0) {
        return;
    }

    const float proporcion = static_cast<float>(vida) / vida_maxima;
    const float ancho_interior = marco_vida.width - 2 * margen_barra_vida;
    const Rectangle relleno{
        marco_vida.x + margen_barra_vida,
        marco_vida.y + margen_barra_vida,
        std::max(1.0f, std::floor(ancho_interior * proporcion)),
        marco_vida.height - 2 * margen_barra_vida,
    };
    const Color color_vida = proporcion <= umbral_vida_baja ? ladrillo : menta;
    DrawRectangleRec(relleno, color_vida);
}

void VistaRaylib::dibujar_pausa(const EstadoPantalla& estado) {
    DrawRectangleRec(lienzo_completo, Fade(BLACK, opacidad_pausa));

    if (estado.confirmar_reinicio) {
        panel(panel_confirmacion);
        titulo("Reiniciar partida", {174, 119}, 22, crema);
        parrafo("Se perderá el avance de la partida actual.", area_aviso_reinicio, 17, crema);
        boton("Volver", area_cancelar_reinicio, Accion::alternar_menu);
        boton("Reiniciar", area_confirmar_reinicio, Accion::confirmar_reinicio);
        return;
    }
    panel(panel_pausa);
    titulo("Pausa", {210, 75}, 24, crema);
    boton("Reanudar", area_reanudar, Accion::alternar_menu);

    if (permite(estado, Accion::guardar_progreso)) {
        boton("Guardar progreso", area_guardar, Accion::guardar_progreso);
    }

    if (permite(estado, Accion::reiniciar_partida)) {
        boton("Reiniciar partida", area_reiniciar, Accion::reiniciar_partida);
    }

    if (permite(estado, Accion::salir_partida)) {
        boton("Salir", area_salir, Accion::salir_partida);
    }
    texto(estado.mensaje_guardado, {210, 272}, 17, crema);
}

std::unique_ptr<Vista> crear_vista_raylib() {
    return std::make_unique<VistaRaylib>();
}

} // namespace veterinaria
