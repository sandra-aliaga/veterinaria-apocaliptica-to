# Rol 5 · Controlador e integración

El quinto frente implementa el controlador y conecta los módulos en
`main.cpp`. La versión de prueba arranca en `Pantalla::inicio` y usa dos
fichas locales y una misión sencilla para recorrer la vista. Al integrar
los módulos, el arranque entregará las fichas del catálogo al consultorio
y construirá la misión con el planificador y el combate. Combate
recibe la fábrica de enemigos y posee el arma. El controlador recibe el
consultorio, la misión y la fábrica de mascotas mediante sus contratos. La
vista conoce la entrada y el estado de pantalla.

El controlador provisional y su formato de guardado están documentados en
la [prueba jugable](../prueba_jugable/README.md).

## Entrega

Implementar [el controlador](controlador.h), los comandos y el arranque del
juego. Implementar también [la vista en raylib](../vista/README.md). En el
arranque, mostrar los errores que informe el catálogo y encolar las fichas
válidas en el orden recibido. Dejar documentado cómo compilar y ejecutar el
proyecto en Windows y Linux al integrar los módulos.

En cada cuadro, leer `EntradaCuadro` desde la vista, actualizar el
controlador y dibujar su `EstadoPantalla` antes de volver a modificarlo.
Para empezar, reemplazar `Consultorio` y `Mision` por versiones simuladas
que devuelvan una ficha, un objetivo y un resultado conocidos.

## Entrada y comandos

| Entrada | Acción |
|---|---|
| W, A, S, D mantenidas | Arriba, izquierda, abajo y derecha. Direcciones opuestas se cancelan y la diagonal mantiene la misma velocidad. |
| Esc al pulsar | Abre o cierra el menú de pausa. Si se está confirmando un reinicio, vuelve al menú. |
| Botón o avance de diálogo | La vista produce la acción indicada por el controlador para esa fase. |
| Contacto con objeto o enemigo | El modelo resuelve la recolección y el combate automáticamente. |

El controlador de prueba valida y despacha las acciones en `actualizar()`.
En el controlador integrado, cada comando llamará a su operación de
`AccionesControlador`. WASD se procesa como la dirección
continua de `EntradaCuadro`. Mientras el menú está abierto, se congelan la
simulación, la recarga y el fundido. En la pausa se puede reanudar, guardar,
reiniciar o salir. El bucle principal cierra la ventana cuando el controlador
emite `salida_solicitada`.

## Guardado de la partida de prueba

El controlador traduce su estado a un registro versionado con el caso,
la fase, la línea de diálogo, el saldo y, si hay una misión, la posición,
el tiempo y los suministros recogidos. [ArchivoPartida](../persistencia/archivo_partida.h)
escribe el registro en `datos/progreso_previa.txt`, usando un archivo temporal
en la misma carpeta antes de reemplazar el guardado. Al pulsar **Jugar**,
el controlador valida el registro y repone el estado. **Nueva partida**
y **Reiniciar partida** conservan el guardado previo hasta la siguiente
escritura explícita. El archivo local queda fuera de Git.

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
`examinar_mascota()` solo cambia la fase y revela los requisitos de la ficha.
El consultorio conserva la ficha en la cola hasta que el jugador decide.

## Ficha y fundido

El consultorio es dueño de la cola y del caso activo. Al presentar un caso,
el controlador copia la ficha a su propio estado. Calcula una vez su edad
equivalente creando la mascota con `FabricaMascotas` y llamando a
`calcularEdadHumana()` a través de `Mascota`. La vista recibe punteros
prestados a los datos que se dibujan en el cuadro.

La copia vive hasta el último diálogo, incluso si el consultorio retira el
caso de la cola o lo cierra al tratarlo. El controlador extrae el resultado
una vez, inicia el fundido, cambia de pantalla y registra el resultado cuando
la pantalla está totalmente oscura. La vista pinta la capa negra según
`opacidad_fundido`, que va de 0 a 1 y vuelve a 0. Mientras sucede esta
transición, la misión ya está detenida.

El futuro `main.cpp` suscribe la vista concreta como
`ReceptorEventosMision` y la da de baja antes de destruirla. La vista puede
reproducir sonidos al recoger o completar. El controlador coordina la
transición usando el resultado de la misión y el HUD lee el progreso del
estado de pantalla.

## Prueba independiente

1. Con un consultorio simulado, recorre llegada, examen y aceptación.
   Verifica que aceptar y rechazar solo aparezcan después de examinar.
2. Haz que la misión simulada termine con éxito. Comprueba que la clínica
   registre el resultado una vez y conserve a los personajes hasta el
   último diálogo.
3. Simula un fallo y verifica que el motivo se muestre y que reintentar
   empiece otro intento con la misma propuesta.
4. Pulsa Esc durante una misión y durante el fundido. Comprueba que se
   congelen ambos y que continúen al reanudar.
