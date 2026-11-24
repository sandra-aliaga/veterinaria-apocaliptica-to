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
