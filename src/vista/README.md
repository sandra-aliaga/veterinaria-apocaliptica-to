# Vista

La vista entrega la entrada al controlador y dibuja a partir de
`EstadoPantalla`. La clínica sigue la composición de una novela visual y
usa una ficha de examen para revisar el caso antes de decidir.

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

El panel del examen solo aparece en la fase `resultado_examen`.
La vista obtiene las cantidades de `ficha->suministros`. Para los recursos
iniciales se muestran los nombres **Vendas** y **Antiséptico** para los
identificadores `vendas` y `antiseptico`. Los rótulos de diálogo salen del
hablante: el nombre del dueño o **Veterinario**. Si no hay una línea activa,
el panel muestra la indicación de la fase. La vista muestra las acciones
que indica `acciones_disponibles`. La acción de examen se envía como
`Accion::examinar`; el controlador cambia de fase.

## Misión y capas de pantalla

El HUD de la misión muestra el progreso de cada suministro y el tiempo
restante en `MM:SS`. Si el intento falla, la clínica indica si se agotó el
tiempo o si el veterinario perdió toda la vida. La vista lee
la orientación horizontal del veterinario y de los enemigos. Con esos datos
elige la hoja izquierda o derecha y avanza los fotogramas si el personaje
camina. El modelo mantiene su orientación al detenerse o al moverse solo
en vertical.

W, A, S y D se traducen a una dirección y Esc alterna el menú. Primero se
dibuja la escena y la capa del fundido con `opacidad_fundido`. Luego se
dibuja el menú encima si está abierto. La vista puede recibir eventos de la
misión para reproducir sonidos.

Para empezar, probar la clínica con una ficha de ejemplo en las fases de
llegada, antes del examen y resultado del examen. Usar figuras provisionales
para el dueño y la mascota mientras se preparan los sprites.
