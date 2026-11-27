# Rol 3 · Catálogo y mascotas

Este frente implementa `CatalogoCasos` para cargar los casos del directorio
de [datos](../../../datos/README.md). Su salida contiene fichas válidas
y los errores de carga asociados a su archivo. Una ficha mal formada se reporta
y las demás pueden seguir cargándose.

| Cabecera | Datos que declara |
|---|---|
| [caso_clinico.h](caso_clinico.h) | La ficha que reúne mascota, dueño, diálogos, suministros y recompensa. |
| [ficha_mascota.h](mascota/ficha_mascota.h) | Identificador, nombre, especie, edad y retrato de la mascota. |
| [ficha_dueno.h](ficha_dueno.h) | Identificador, nombre y sprite del dueño. |
| [cantidad_suministro.h](cantidad_suministro.h) | Identificador y cantidad, que la misión también usa para el objetivo y el inventario. |
| [dialogos_caso.h](dialogos_caso.h) | Las listas de diálogo por etapa del caso. |
| [linea_dialogo.h](linea_dialogo.h) | Una línea de diálogo y quién la dice. |

Crear primero dos fichas. Una puede pedir un solo tipo de suministro y la otra
varios, con cantidades distintas. Probar que cada ficha conserve todos los
diálogos y rutas de arte. El retrato, el sprite y el diálogo son datos
configurables para cada caso.

Las cabeceras de la familia están en `mascota/`. `FichaMascota` conserva
los datos del JSON por valor, incluida la edad en meses.
[`FabricaMascotas`](mascota/fabrica_mascotas.h) crea un `Gato` o un `Perro` a partir de la
especie. Ambos heredan de `Mascota` y declaran su propia implementación de
`calcularEdadHumana()`, como en la práctica 03. El controlador usa ese método
al preparar la ficha para mostrarla. Agregar otro gato o perro solo requiere
agregar sus datos y su imagen.

## Entrega

- Implementar `CatalogoCasos::cargar()` para leer fichas UTF-8 en orden de
  nombre de archivo. Usar el [formato compartido](../../../datos/README.md).
  Entregar fichas válidas al rol 5 mediante `CargaCasos`. Para cada archivo
  inválido, registrar su nombre y el campo o motivo del error.
- Implementar `Mascota`, `Gato`, `Perro` y `FabricaMascotas`. Probar el cálculo
  de edad llamándolo a través de un `std::unique_ptr<Mascota>`. Para inicializar
  la jerarquía, usar los datos de `FichaMascota` y el comportamiento de la
  práctica 03.
- Entregar dos JSON jugables, uno de cada especie. Una ficha pide un tipo
  de suministro y la otra pide varios. Al menos una ficha debe incluir
  varias líneas de diálogo en la misma etapa.
- Coordinar los PNG de los JSON y los sprites del juego con el rol 5.
  El inventario de archivos y el flujo de generación están en la
  [guía de arte](../../../arte/README.md).

## Prueba independiente

Cargar las dos fichas desde una carpeta de ejemplo y comprobar el orden,
los retratos y las cantidades. Agregar un tercer archivo con un suministro
mal definido y verificar que se informe ese archivo y que las otras dos
fichas sigan disponibles. Al cambiar la cantidad de un suministro en el JSON,
comprobar que el catálogo entregue el nuevo valor.

## Implementación

| Clase | Archivo | Qué resuelve |
|---|---|---|
| `CatalogoCasosJson` | [`catalogo_casos_json.h`](catalogo_casos_json.h) | Implementa `CatalogoCasos`. Lee cada `.json` en orden de nombre, valida la ficha y reporta los errores por archivo. |
| `FabricaMascotasBase` | [`mascota/fabrica_mascotas_base.h`](mascota/fabrica_mascotas_base.h) | Crea un `Gato` o un `Perro` según la especie y devuelve nulo para las demás. |
| `ValorJson` y `leer_json` | [`json/valor_json.h`](json/valor_json.h) | Lector JSON propio, sin dependencias. Admite UTF-8, la marca BOM y escapes `\u`, e informa la línea y la columna de cada error de sintaxis. |

Integración desde el controlador:

```cpp
const FabricaMascotasBase fabrica;
const CatalogoCasosJson catalogo(fabrica);
const CargaCasos carga = catalogo.cargar("datos");
// carga.casos va a Consultorio::recibir(); carga.errores se muestra al equipo.
```

El catálogo recibe la fábrica de mascotas para validar la especie. Admitir
una especie nueva solo requiere ampliar la fábrica.

### Edad humana

| Edad de la mascota | Gato | Perro |
|---|---|---|
| Primer año | 15 años, proporcional por mes | 15 años, proporcional por mes |
| Segundo año | suma 9 (24 al cumplir 2) | suma 9 (24 al cumplir 2) |
| Cada año siguiente | suma 4 | suma 5 |

Mora (24 meses) equivale a 24 años y Toby (48 meses) a 34.

### Errores de carga

Cada `ErrorCargaCaso` indica el archivo y el campo con la ruta completa, por
ejemplo `suministros[0].cantidad: debe ser un entero mayor o igual a 1`. Se
informan el JSON mal formado (con línea y columna), los campos ausentes o de
otro tipo, los textos vacíos, la especie no admitida, el hablante desconocido,
los suministros repetidos o sin cantidad positiva, las rutas con `\` y los
identificadores de caso repetidos entre archivos.

## Pruebas en consola

Desde la raíz del repositorio:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_catalogo \
    pruebas/pruebas_catalogo.cpp src/modelo/catalogo/*.cpp \
    src/modelo/catalogo/json/*.cpp src/modelo/catalogo/mascota/*.cpp
./build/pruebas_catalogo
```
