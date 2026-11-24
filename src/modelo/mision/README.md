# Misión

El frente de mapa implementa también la coordinación de la misión. El
[planificador](mapa/README.md) prepara el terreno y las oleadas. El
[combate](combate/README.md) actualiza al veterinario y a los enemigos.

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
