# Rol 2 · Mapa y oleadas

`PlanificadorMapa` recibe la propuesta con todos los suministros requeridos.
Prepara el inicio, los límites, los obstáculos y posiciones alcanzables para
recoger el objetivo. Puede repartir varias unidades de un suministro entre
distintas posiciones. El origen de coordenadas queda arriba a la izquierda y
el tiempo se mide en segundos.

El rol 2 implementa [`PlanificadorCampoAbierto`](planificador_campo_abierto.h),
que hereda de `PlanificadorMapa`. La misión usa la interfaz y puede trabajar
con otro planificador que implemente el mismo contrato. La clase concreta
encapsula los límites, el generador aleatorio y el estado de las oleadas.
Para repetir una prueba, puede recibir una semilla fija. Con los mismos datos
y semilla, `preparar()` debe producir la misma distribución en la misma
compilación. En juego se puede omitir la semilla y variar la distribución.

[`preparar()`](planificador_mapa.h) devuelve el terreno de un intento nuevo y
reinicia el reloj de las oleadas. `avanzar(segundos, jugador, terreno)` devuelve
las apariciones que vencen en ese paso, con su tipo y posición. La [misión](../README.md) las
entrega al rol 4; el planificador puede probarse sin construir enemigos.

La primera oleada programa 50 enemigos a los 2 s. Durante el primer minuto
llegan animales de contacto cada 10 s. Más adelante se suman supervivientes
de contacto y tiradores, y el intervalo baja a 8 s y luego a 6 s.

Cada oleada reparte sus apariciones entre los cuatro lados de la cámara.
Las posiciones se eligen según el terreno y la ubicación actual del
veterinario. Si un lado queda fuera del campo, se prueba otro. Los tiradores
prefieren ubicarse cerca de suministros pendientes cuando pueden hacerlo
fuera de cámara. El combate mantiene hasta 200 enemigos activos.
Cada suministro pendiente activa un anillo de hasta seis vigías cuando el
veterinario se aproxima a 500 px. Los puestos quedan a 72–96 px del objeto.
Si un puesto cae dentro de cámara o sobre una colisión, el planificador
busca otra posición en el mismo sector. La guardia mantiene su puesto y
se activa una sola vez por suministro e intento.

## Campo abierto

La demo empieza en un claro dentro de un campo amplio. Al preparar cada
intento, distribuir árboles, rocas o troncos dentro de `Terreno.limites`.
Dejar despejado el inicio y suficiente espacio entre obstáculos para esquivar
enemigos en varias direcciones. En el campo de 3072 × 2048 px los suministros
se colocan en zonas alcanzables, entre 960 y aproximadamente 1640 px del punto de inicio.
El planificador busca repartirlos con al menos 360 px entre unidades.

Cada [`ObstaculoMapa`](obstaculo_mapa.h) tiene un `tipo` (`arbol`, `roca` o
`tronco`), un `anclaje` y un rectángulo de `colision`, todos en coordenadas del
mundo. El rol 4 usa `colision` para bloquear el paso y el rol 5 elige el PNG
por `tipo` y coloca su punto de apoyo en `anclaje`. Para el árbol, el anclaje
y la colisión quedan en la base del tronco. La copa puede ocupar más espacio.

## Suelo

`Terreno.suelo` es un [`SueloMapa`](suelo_mapa.h): una cuadrícula de piezas de
16 × 16 px que cubre `Terreno.limites`, fila por fila desde arriba a la
izquierda. Cada [`PiezaSuelo`](suelo_mapa.h) es `pasto`, `tierra` o
`transicion`, y el rol 5 dibuja el PNG que corresponde. El PNG de transición
tiene el pasto arriba y la tierra abajo. `giro_cuartos` indica cuántos giros
horarios de 90° hay que darle para que la tierra quede del lado correcto
(0 abajo, 1 izquierda, 2 arriba, 3 derecha). El claro del inicio es tierra y
otros claros de tierra se reparten por el campo. Si `piezas` viene vacío, la
vista puede usar solo pasto.

La hierba baja y los restos pequeños son decoración. Los dos casos comparten
las piezas de terreno, con distribuciones y objetivos que pueden variar.
[Guía visual del mapa](../../../../arte/README.md#mapa-de-la-demo)

## Prueba independiente

Prepara un caso con dos vendas y un antiséptico. Comprueba que el claro quede
libre, que el mapa ofrezca esas cantidades y que se pueda rodear un grupo
de obstáculos sin perder el acceso a los suministros. Prueba más de una
distribución y repite una con semilla fija. Avanza el reloj en pasos fijos y
verifica que cada aparición se entregue en el paso que corresponde. Cambia la
posición del veterinario entre pasos y comprueba que las apariciones salgan
fuera de su cámara, por varios lados y en zonas libres, y que se omitan cuando
no hay lugar. Revisa que el suelo tenga pasto, tierra y transiciones con el
giro correcto.
