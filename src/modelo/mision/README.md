# Rol 2 · Misión

El frente de mapa implementa también la coordinación de la misión. El
[planificador](mapa/README.md) prepara el terreno y las oleadas. El
[combate](combate/README.md) actualiza al veterinario y a los enemigos.

| Operación de [`Mision`](mision.h) | Qué debe hacer |
|---|---|
| `iniciar(propuesta)` | Guardar el ID y los requisitos, preparar el terreno, iniciar combate y reiniciar reloj e inventario. |
| `actualizar(direccion, segundos)` | Pedir apariciones al planificador, pasarlas a combate y resolver la recolección y el final usando su estado. |
| `estado()` | Exponer el terreno, los objetos que quedan, el combate, los requisitos, lo recogido y el tiempo restante. |
| `extraer_resultado()` | Devolver una sola vez el desenlace del caso y las cantidades recogidas por ID. |

Cada intento empieza con el tiempo de `PropuestaMision::limite_segundos`.
El límite de prueba es de 300 segundos. El reloj avanza durante el combate
y se congela al pausar y al terminar el intento.

En cada cuadro, la misión recibe las apariciones pendientes y actualiza el
combate hasta el tiempo restante. Si la vida del veterinario llega a cero,
registra `sin_vida` antes de procesar la recolección. Si sigue vivo, recoge
los suministros por contacto, acumula las cantidades por identificador y
retira cada objeto consumido. Al reunir todo, termina con éxito. Si se agota
el tiempo con suministros pendientes, registra `tiempo_agotado`. Recoger
el último suministro en el último instante cuenta como éxito si sigue vivo.

El resultado se puede extraer una sola vez. La misión queda detenida y el
controlador inicia el fundido de regreso a la clínica. Al reintentar se
regenera el intento con vida, enemigos, suministros y reloj reiniciados.
Un resultado fallido indica el motivo. En un resultado completado se deja vacío.

Para Observer, el modelo emite `suministro_recogido` después de actualizar
el inventario e `intento_finalizado` una vez al cerrar el intento. Si una
recolección completa el objetivo, emite primero el aviso de recolección.

## Prueba independiente

1. Con un combate simulado, coloca al veterinario sobre dos objetos del
   mismo tipo. Verifica que se acumulen las unidades y que cada objeto se
   retire del terreno una sola vez.
2. Deja pendiente un suministro. Comprueba que el intento siga activo hasta
   recogerlo o agotar el tiempo. Al completarlo, extrae el resultado dos
   veces y comprueba que la segunda devuelva vacío.
3. Prueba los límites del último cuadro. Con vida cero se registra un fallo.
   Con vida y el último suministro recogido a tiempo se registra éxito.
4. Inicia otro intento y comprueba que vuelva a tener sus objetos y su tiempo.
