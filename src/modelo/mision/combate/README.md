# Rol 4 · Combate

El frente de combate recibe un terreno y solicitudes de aparición. Implementa
`VeterinarioJugable`, que conserva posición, vida, velocidad y orientación.
Combate resuelve el movimiento, las colisiones y el daño. Puede probarlos con
un terreno fijo y una oleada de ejemplo.

## Entrega a la misión

| Operación de [`Combate`](combate.h) | Qué debe hacer |
|---|---|
| `iniciar(terreno)` | Colocar al veterinario en el inicio, restaurar su vida, limpiar enemigos y proyectiles y reiniciar el arma. |
| `agregar_enemigo(aparicion)` | Crear el enemigo del tipo y la posición indicados por el rol 2. |
| `actualizar(direccion, segundos)` | Mover al jugador, resolver el terreno, ejecutar estrategias, aplicar ataques y retirar las entidades eliminadas. |
| `estado()` | Exponer posición, vida y movimiento del veterinario, estado de cada enemigo y posiciones de proyectiles. |

## Arma automática

Las cabeceras de armas están en `arma/`. La implementación de combate posee
un `std::unique_ptr<Arma>`. La primera arma es [`ArmaBase`](arma/arma_base.h).
Su alcance adicional es cero y el golpe se activa cuando
un enemigo está en contacto. Si hay varios, elige el más cercano y golpea a
uno por intervalo. El alcance se mide desde los límites de contacto.
`Arma` expone el alcance, el avance de la recarga y el intento de ataque. Así
otra arma puede usar alcance o cadencia distintos desde el mismo contrato.
Cada intento inicia con el arma lista.

## Enemigos

Las cabeceras de enemigos y sus estrategias están en `enemigo/`.
La [fábrica](enemigo/fabrica_enemigos.h) crea un enemigo a partir de una aparición y transfiere su propiedad
a combate con `std::unique_ptr<Enemigo>`. `EnemigoConEstrategia` hereda de
`Enemigo` y posee el estado común y su estrategia. `PerseguirPorContacto` y
`AtacarADistancia` implementan el mismo contrato. Combate aplica el daño por
contacto y administra los proyectiles del enemigo a distancia.

| Tipo en `Aparicion` | Quién es | Estrategia |
|---|---|---|
| `animal_de_contacto` | Animal agresivo que corre hacia el veterinario. | `PerseguirPorContacto` |
| `superviviente_de_contacto` | Superviviente hostil que se acerca a disputar los suministros. | `PerseguirPorContacto` |
| `superviviente_a_distancia` | Superviviente hostil que mantiene distancia. | `AtacarADistancia` |

La fábrica elige la estrategia por tipo. La vista usa ese mismo tipo para
dibujar sprites distintos aunque dos enemigos se comporten de forma parecida.

El combate entrega el estado final de cada cuadro. La misión consulta este
estado para resolver el fallo y, si el veterinario sigue vivo, la recolección
de suministros.

## Prueba independiente

1. Usa un terreno con un obstáculo y dos apariciones manuales. Verifica que el
   jugador respete los límites y que al detenerse mantenga la última orientación.
2. Acerca un enemigo de contacto al veterinario y comprueba el daño y la
   recarga del arma. Si hay dos objetivos en contacto, verifica que el arma
   seleccione al más cercano y solo golpee a uno por intervalo.
3. Coloca un superviviente a distancia y verifica que mantenga distancia y
   genere proyectiles. Comprueba que retirarlos y volver a iniciar el combate
   deje el estado listo para otro intento.
