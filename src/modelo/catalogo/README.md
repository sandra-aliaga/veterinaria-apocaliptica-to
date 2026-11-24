# Catálogo

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
