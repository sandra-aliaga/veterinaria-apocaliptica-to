# Rol 5 · Vista en raylib

La vista entrega la entrada al controlador y dibuja a partir de
`EstadoPantalla`. La clínica sigue la composición de una novela visual y
usa una ficha de examen para revisar el caso antes de decidir.

La implementación en [raylib](vista_raylib.cpp) tiene tres pantallas.
Convierte botones y teclas en `EntradaCuadro` y dibuja `EstadoPantalla`.
Lee de ahí la ficha, el diálogo, el saldo, el tiempo y las acciones válidas.
El [controlador del juego](../controlador/controlador_juego.h) entrega
los casos del catálogo y el estado de la misión activa.

## Inicio

La vista carga el fondo nocturno, el título, el botón **Jugar** y el cursor
desde `arte/inicio/`. Al pulsar **Jugar**, la vista envía
`Accion::iniciar_partida` y el controlador presenta el primer caso.
La vista conserva las coordenadas de 640 × 360, dibuja al doble de
resolución y ajusta la imagen al área 16:9 más grande de la ventana.
Usa Handjet para diálogos, botones y datos, y Pixelify Sans para
encabezados. El título principal se carga como PNG.
[Licencia de Handjet](../../arte/fuentes/OFL-Handjet.txt)
[Licencia de Pixelify Sans](../../arte/fuentes/OFL-PixelifySans.txt)

Las rutas fijas, los tamaños y la composición se declaran al inicio de
[`vista_raylib.cpp`](vista_raylib.cpp). Las fuentes y las texturas fijas
se cargan al crear la vista. Los retratos vienen del estado de pantalla y
se cargan una vez por ruta antes de dibujarlos.

## Clínica

Tomar una pantalla horizontal como referencia. El fondo de la clínica
improvisada se mantiene durante toda la atención. En la zona izquierda aparece
el sprite del dueño con su nombre. A su lado, en una mesa, aparece el retrato o sprite de
la mascota con su nombre. Abajo, un panel ancho muestra quién habla, el texto
de la línea actual y la acción disponible. A la derecha se reserva espacio
para la ficha de examen.

Boceto de la pantalla después de examinar a Mora:

```text
┌───────────────────────────────────────┬───────────────────────┐
│ Consultorio                           │ Ficha de examen       │
│                                       │                       │
│     [Elena]           [Mora]          │ Mora · Gato           │
│      Dueña        Mesa de examen      │ Edad: 24 meses        │
│                                       │                       │
│                                       │ Suministros           │
│                                       │ Vendas × 2            │
│                                       │ Antiséptico × 1       │
│                                       │                       │
│                                       │ [Aceptar] [Rechazar]  │
├───────────────────────────────────────┴───────────────────────┤
│ Revisa los suministros y decide si aceptas el caso.           │
└───────────────────────────────────────────────────────────────┘
```

| Momento | Qué muestra la vista | Qué puede hacer el jugador |
|---|---|---|
| Llegada | Dueño, mascota y diálogo inferior. | Pulsar **Continuar** para avanzar una línea. |
| Antes del examen | Los personajes siguen visibles y el panel inferior ofrece examinar. | Pulsar **Examinar mascota**. |
| Resultado del examen | Ficha lateral con nombre, especie, edad en meses y edad equivalente si está disponible. Debajo se muestran los suministros con sus cantidades. | Pulsar **Aceptar** o **Rechazar** dentro de la ficha. |
| Aceptación o rechazo | Se cierra la ficha. Los personajes siguen visibles durante el diálogo. | Avanzar el diálogo para salir a la misión o presentar el siguiente caso. |
| Regreso a la clínica | Vuelven a verse el dueño y la mascota durante el diálogo de tratamiento o fallo. | Pasar a la siguiente ficha al acabar el tratamiento o **Reintentar** tras un fallo. |
| Jornada terminada | Consultorio vacío y mensaje **No quedan casos**. | Abrir el menú. |

El panel del examen solo aparece en la fase `resultado_examen` e incluye
el retrato de la mascota dentro de la ficha.
En el lienzo de 640 × 360, el dueño ocupa 120 × 160 px, la mascota en la
mesa 84 × 84 px y su retrato dentro de la ficha 88 × 88 px. Las figuras
se amplían hacia arriba para conservar su apoyo en la escena.
La vista obtiene las cantidades de `ficha->suministros`. Para los recursos
iniciales se muestran los nombres **Vendas** y **Antiséptico** para los
identificadores `vendas` y `antiseptico`. Los rótulos de diálogo salen del
hablante: el nombre del dueño o **Veterinario**. Si no hay una línea activa,
el panel muestra la indicación de la fase. La vista muestra las acciones
que indica `acciones_disponibles`. La acción de examen se envía como
`Accion::examinar`; el controlador cambia de fase.

## Misión y capas de pantalla

El HUD de la misión muestra el progreso de cada suministro, el tiempo
restante en `MM:SS` y una barra con la vida actual y máxima del veterinario.
La barra cambia a rojo cuando queda un cuarto de vida o menos. Al recoger
un objeto aparece un aviso. Cuando el suministro más cercano queda fuera
de cámara, una flecha indica su dirección. Si el intento falla, la clínica
indica si se agotó el tiempo o el veterinario perdió toda la vida.

La vista dibuja las baldosas visibles de `Terreno.suelo` y gira las
transiciones según su orientación. Los obstáculos y suministros usan sus
PNG, y la hierba seca y los escombros se distribuyen como decoración.
El modelo resuelve las colisiones.

El veterinario y los enemigos usan hojas de cuatro fotogramas por dirección.
El vigía tiene una animación de lanzamiento y el rayo gira según su
velocidad. El daño, el golpe del látigo y la recogida tienen efectos
breves ligados a la simulación, que se congelan al pausar.
[Rutas y prompts del campo](../../arte/README.md#prompts-del-suelo-y-los-obstáculos)

W, A, S y D se traducen a una dirección y Esc alterna el menú. Primero se
dibuja la escena y la capa del fundido con `opacidad_fundido`. Luego se
dibuja el menú encima si está abierto. La vista puede recibir eventos de la
misión para reproducir sonidos.

## Prueba independiente

Prepara estados de ejemplo para llegada, antes del examen y resultado del
examen. Usa figuras provisionales para el dueño y la mascota. Comprueba que
el panel lateral aparezca solo al examinar, que muestre las cantidades de la
ficha y que los botones generen la acción correcta. Prueba también un estado
de misión con tiempo, enemigos, suministros parciales y un menú abierto.
