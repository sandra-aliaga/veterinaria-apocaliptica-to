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
El daño se configura por tipo de enemigo y el estado lo expone en
`EstadoEnemigo::danio`. Combate aplica el daño y comunica la vida actual
y máxima del veterinario para dibujar la barra en el HUD.

### Ráfaga del vigía

Como próximo ajuste de combate, el vigía hará una pausa breve antes de
lanzar varios rayos de energía en abanico. Cada rayo se mueve y colisiona
por separado. Los rayos deben dejar huecos para esquivar la ráfaga.
Para dibujarlos, el estado del proyectil deberá incluir su dirección de
vuelo además de la posición. Al acertar se retira el rayo y el HUD refleja
la pérdida de vida. [Prompt del rayo](../../../../arte/prompts/mision/enemigos/vigia_rayo.txt)

El combate entrega el estado final de cada cuadro. La misión consulta este
estado para resolver el fallo y, si el veterinario sigue vivo, la recolección
de suministros.

Para resolver el terreno, usar el rectángulo `colision` de cada
`ObstaculoMapa`. Un árbol bloquea el paso en la base del tronco. El sprite
puede tener una copa mayor que esa zona sólida.

## Prueba independiente

1. Usa un terreno con un obstáculo y dos apariciones manuales. Verifica que el
   jugador respete los límites y que al detenerse mantenga la última orientación.
2. Acerca un enemigo de contacto al veterinario y comprueba el daño y la
   recarga del arma. Si hay dos objetivos en contacto, verifica que el arma
   seleccione al más cercano y solo golpee a uno por intervalo.
3. Coloca un superviviente a distancia y verifica que mantenga distancia y
   genere proyectiles. Comprueba que retirarlos y volver a iniciar el combate
   deje el estado listo para otro intento.

## Implementación

| Clase | Archivo | Qué resuelve |
|---|---|---|
| `CombateBase` | [`combate_base.h`](combate_base.h) | Implementa `Combate`. Posee la fábrica, el arma, el veterinario, los enemigos y los proyectiles. |
| `ConfiguracionCombate` | [`combate_base.h`](combate_base.h) | Vida y velocidad del veterinario, radios de contacto, recarga del daño por contacto y datos de los proyectiles. |
| `FabricaEnemigosBase` | [`enemigo/fabrica_enemigos_base.h`](enemigo/fabrica_enemigos_base.h) | Crea cada enemigo con el perfil de su tipo y le asigna su estrategia. |

Integración desde la misión:

```cpp
auto combate = std::make_unique<CombateBase>(std::make_unique<FabricaEnemigosBase>(),
                                             std::make_unique<ArmaBase>(10, 0.5F));
```

Valores iniciales de cada tipo:

| Tipo | Vida | Velocidad | Daño |
|---|---|---|---|
| `animal_de_contacto` | 30 | 90 | 10 por contacto |
| `superviviente_de_contacto` | 60 | 60 | 15 por contacto |
| `superviviente_a_distancia` | 40 | 50 | 8 por proyectil, alcance 220, cada 2 s |

Cada cuerpo es un círculo. El movimiento se resuelve por ejes para que el
veterinario se deslice junto a los obstáculos, y los pasos de más de 0,05 s
se dividen. Un enemigo de contacto hace daño como máximo una vez cada 0,5 s.
Con la vida en cero el combate deja de avanzar.

La ráfaga del vigía queda pendiente: `EstadoCombate::proyectiles` todavía
expone solo posiciones.

## Pruebas en consola

Desde la raíz del repositorio:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_combate \
    pruebas/pruebas_combate.cpp src/modelo/mision/combate/*.cpp \
    src/modelo/mision/combate/arma/*.cpp src/modelo/mision/combate/enemigo/*.cpp
./build/pruebas_combate
```
