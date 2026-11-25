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
reinicia el reloj de las oleadas. `avanzar(segundos)` devuelve las apariciones
que vencen en ese paso, con su tipo y posición. La [misión](../README.md) las
entrega al rol 4; el planificador puede probarse sin construir enemigos.

Para la prueba de cinco minutos, empezar con grupos pequeños de animales
durante el primer minuto. Después pueden aparecer supervivientes de contacto
y, en las oleadas densas del tramo final, supervivientes a distancia cerca
de los suministros. Los enemigos aparecen en el
perímetro y dejan espacio alrededor del veterinario. El objetivo se puede
recoger desde el inicio. En la primera prueba, buscar que un recorrido
directo se pueda completar en dos o tres minutos y ajustar el tamaño y la
frecuencia de los grupos según cómo se juega.

## Campo abierto

La demo empieza en un claro dentro de un campo amplio. Al preparar cada
intento, distribuir árboles, rocas o troncos dentro de `Terreno.limites`.
Dejar despejado el inicio y suficiente espacio entre obstáculos para esquivar
enemigos en varias direcciones. Colocar los suministros en zonas libres y
comprobar que sean alcanzables desde el claro.

Cada [`ObstaculoMapa`](obstaculo_mapa.h) tiene un `tipo` (`arbol`, `roca` o
`tronco`), un `anclaje` y un rectángulo de `colision`, todos en coordenadas del
mundo. El rol 4 usa `colision` para bloquear el paso y el rol 5 elige el PNG
por `tipo` y coloca su punto de apoyo en `anclaje`. Para el árbol, el anclaje
y la colisión quedan en la base del tronco. La copa puede ocupar más espacio.

La hierba baja y los restos pequeños son decoración. Los dos casos comparten
las piezas de terreno, con distribuciones y objetivos que pueden variar.
[Guía visual del mapa](../../../../arte/README.md#mapa-de-la-demo)

## Prueba independiente

Prepara un caso con dos vendas y un antiséptico. Comprueba que el claro quede
libre, que el mapa ofrezca esas cantidades y que se pueda rodear un grupo
de obstáculos sin perder el acceso a los suministros. Prueba más de una
distribución y repite una con semilla fija. Avanza el reloj en pasos fijos y
verifica que cada aparición se entregue en el paso que corresponde.
