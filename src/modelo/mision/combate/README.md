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
El látigo se acciona cada 0,65 s hacia el lado que mira el veterinario, incluso
si el barrido queda vacío. Su alcance frontal es de 108 px y daña a todos los
enemigos que alcancen el área de golpe. La vista usa ese mismo alcance para
dibujar la animación. Cada intento empieza con el arma lista.
`Arma` expone el alcance, el avance de la recarga y el intento de ataque.

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

Cada disparo del vigía abre un abanico de cinco rayos separados 16°.
El modelo expone el origen, la posición y la velocidad de cada rayo.
El tramo recorrido entero hace daño. El rayo desaparece al tocar al
veterinario, alcanzar un obstáculo o agotar su duración.
Los vigías de la guardia conservan su puesto alrededor del suministro. Los
vigías de las oleadas se retiran por debajo de 99 px. Ambos caen con dos
latigazos: tras el primero conservan 8 de sus 40 puntos de vida y aparece
su barra.
[Prompt del rayo](../../../../arte/prompts/mision/enemigos/vigia_rayo.txt)

El combate entrega el estado final de cada cuadro. La misión consulta este
estado para resolver el fallo y, si el veterinario sigue vivo, la recolección
de suministros.

Para resolver el terreno, usar el rectángulo `colision` de cada
`ObstaculoMapa`. Un árbol bloquea el paso en la base del tronco. El sprite
puede tener una copa mayor que esa zona sólida.

## Prueba independiente

1. Usa un terreno con un obstáculo y dos apariciones manuales. Verifica que el
   jugador respete los límites y que al detenerse mantenga la última orientación.
2. Comprueba que el látigo se anime con el área vacía. Coloca varios enemigos
   a distintos lados del veterinario y verifica que un barrido dañe a todos
   los que están al frente y al alcance.
3. Coloca un superviviente a distancia y verifica que mantenga distancia y
   genere proyectiles. Comprueba que retirarlos y volver a iniciar el combate
   deje el estado listo para otro intento.

## Implementación

| Clase | Archivo | Qué resuelve |
|---|---|---|
| `CombateBase` | [`combate_base.h`](combate_base.h) | Implementa `Combate`. Posee la fábrica, el arma, el veterinario, los enemigos y los proyectiles. |
| `ConfiguracionCombate` | [`combate_base.h`](combate_base.h) | Vida y velocidad del veterinario, radios de contacto, protección tras recibir daño y datos de los proyectiles. |
| `FabricaEnemigosBase` | [`enemigo/fabrica_enemigos_base.h`](enemigo/fabrica_enemigos_base.h) | Crea cada enemigo con el perfil de su tipo y le asigna su estrategia. |

Integración desde la misión:

```cpp
auto combate = std::make_unique<CombateBase>(std::make_unique<FabricaEnemigosBase>(),
                                             std::make_unique<ArmaBase>(32, 0.65F, 108.0F));
```

Valores iniciales de cada tipo:

| Tipo | Vida | Velocidad | Daño |
|---|---|---|---|
| `animal_de_contacto` | 30 | 80 | 10 por contacto |
| `superviviente_de_contacto` | 60 | 54 | 15 por contacto |
| `superviviente_a_distancia` | 40 | 45 | 8 por proyectil, alcance 220, cada 2 s |

Cada cuerpo es un círculo. El movimiento se resuelve por ejes para que el
veterinario se deslice junto a los obstáculos, y los pasos de más de 0,05 s
se dividen. Tras recibir daño por contacto o proyectil, el veterinario tiene
0,45 s de protección. Cada baja por látigo recupera 1 punto de vida hasta
el máximo. Con la vida en cero el combate deja de avanzar.

## Pruebas en consola

Desde la raíz del repositorio:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_combate \
    pruebas/pruebas_combate.cpp src/modelo/mision/combate/*.cpp \
    src/modelo/mision/combate/arma/*.cpp src/modelo/mision/combate/enemigo/*.cpp
./build/pruebas_combate
```
