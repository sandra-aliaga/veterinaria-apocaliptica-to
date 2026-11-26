# Rol 5 · Controlador e integración

[`main.cpp`](../main.cpp) carga las fichas JSON y construye el consultorio,
el planificador, el combate, la misión y la vista. [`ControladorJuego`](controlador_juego.h)
recibe los contratos del consultorio, la misión y la fábrica de mascotas.
En cada cuadro entrega el estado a la vista.

Al aceptar, el consultorio mueve la ficha de la cola al caso activo y
entrega los requisitos. Al terminar el diálogo, el controlador inicia la
misión. Recibe por Observer los avisos de recolección y fin del intento.
Registra el resultado una vez en el consultorio y hace el fundido de
regreso. Conserva la ficha leída del catálogo para mostrar el diálogo
final aunque el consultorio ya haya cerrado el caso.

Los ajustes compartidos están en [`configuracion_juego.h`](configuracion_juego.h).
[`guardado_partida.cpp`](guardado_partida.cpp) conserva el caso por su ID,
el saldo y la fase. Un guardado de combate reanuda desde el inicio del
intento, con vida y reloj restaurados.

## Entrada y comandos

| Entrada | Acción |
|---|---|
| W, A, S, D mantenidas | Arriba, izquierda, abajo y derecha. Direcciones opuestas se cancelan y la diagonal mantiene la misma velocidad. |
| Esc al pulsar | Abre o cierra el menú de pausa. Si se está confirmando un reinicio, vuelve al menú. |
| Botón o avance de diálogo | La vista produce la acción indicada por el controlador para esa fase. |
| Contacto con objeto o enemigo | El modelo resuelve la recolección y el combate automáticamente. |

El controlador valida y despacha las acciones en `actualizar()`.
Cada acción llama a su operación de `AccionesControlador`. WASD se procesa como la dirección
continua de `EntradaCuadro`. Mientras el menú está abierto, se congelan la
simulación, la recarga y el fundido. En la pausa se puede reanudar, guardar,
reiniciar o salir. El bucle principal cierra la ventana cuando el controlador
emite `salida_solicitada`.

## Guardado de la partida

[ArchivoPartida](../persistencia/archivo_partida.h) escribe el registro
versionado en `datos/progreso_previa.txt`. El archivo conserva el saldo, el
caso, la fase y la línea. Si la partida está en combate, el siguiente inicio
carga un nuevo intento para ese caso. Durante el fundido de regreso se
guarda el resultado ya registrado. **Nueva partida** y **Reiniciar partida**
conservan el archivo anterior hasta que se vuelva a guardar desde la pausa.

## Fases y acciones válidas

| Fase | Acciones adicionales a Esc | Qué ocurre al terminar |
|---|---|---|
| `portada` | Jugar | Presenta el primer caso. |
| `sin_casos` | Ninguna | Se muestra que la jornada terminó. |
| `llegada` | Avanzar diálogo | Se habilita Examinar mascota. |
| `por_examinar` | Examinar mascota | El controlador pasa a `resultado_examen` y la vista abre la ficha. |
| `resultado_examen` | Aceptar o rechazar | La ficha muestra los datos de la mascota y los suministros que pide. Al aceptar se muestra su diálogo y se inicia la misión. Al rechazar se muestra su diálogo y se pasa a la siguiente ficha. |
| `aceptacion` y `rechazo` | Avanzar diálogo | Se ejecuta la transición elegida. |
| `jugando` | Movimiento continuo | Al terminar el intento, se inicia el fundido. |
| `regresando` | Ninguna | A oscuridad completa cambia a la clínica. Al aclararse, se muestra el diálogo de tratamiento o fallo. |
| `fallo` | Avanzar diálogo | Se muestra el motivo del fallo y se habilita reintentar. |
| `reintento` | Reintentar | Vuelve al mapa con el objetivo original. |
| `tratamiento` | Avanzar diálogo | Se presenta la siguiente ficha o se cierra la jornada. |

Las etapas con diálogos vacíos se saltan. `acciones_disponibles` indica lo
que puede ofrecer la vista y lo que el controlador acepta en ese cuadro.
`examinar_mascota()` calcula la edad equivalente y revela los requisitos.
El consultorio conserva la ficha en la cola hasta que el jugador decide.

## Ficha y fundido

El consultorio posee la cola y el caso activo. El controlador mantiene las
fichas del catálogo y calcula la edad equivalente mediante la fábrica de
mascotas al examinar. La vista recibe punteros prestados a los datos
necesarios para dibujar el cuadro.

La misión se detiene al publicar el resultado. El controlador lo registra
en el consultorio y sube `opacidad_fundido` de 0 a 1. A oscuridad completa
cambia a la clínica y baja la opacidad de vuelta a 0.

## Prueba independiente

1. Recorre llegada, examen y aceptación.
   Verifica que aceptar y rechazar solo aparezcan después de examinar.
2. Completa la misión recogiendo todos los suministros. Comprueba que la clínica
   registre el resultado una vez y conserve a los personajes hasta el
   último diálogo.
3. Simula un fallo y verifica que el motivo se muestre y que reintentar
   empiece otro intento con la misma propuesta.
4. Pulsa Esc durante una misión y durante el fundido. Comprueba que se
   congelen ambos y que continúen al reanudar.
