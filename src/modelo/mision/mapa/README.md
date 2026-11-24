# Rol 2 · Mapa y oleadas

`PlanificadorMapa` recibe la propuesta con todos los suministros requeridos.
Prepara el inicio, los límites, los obstáculos y posiciones alcanzables para
recoger el objetivo. Puede repartir varias unidades de un suministro entre
distintas posiciones. El origen de coordenadas queda arriba a la izquierda y
el tiempo se mide en segundos.

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

## Prueba independiente

Prepara un caso con dos vendas y un antiséptico. Comprueba que el mapa ofrece
esas cantidades y que se puede llegar a ellas desde el inicio sin atravesar
un obstáculo. Avanza el reloj en pasos fijos, verifica que una aparición se
entregue solo en el paso que corresponde y repite desde `preparar()`.
