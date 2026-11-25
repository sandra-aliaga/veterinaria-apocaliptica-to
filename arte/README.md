# Guía de arte · Veterinaria apocalíptica

La clínica, el mapa, los personajes, los objetos y la interfaz siguen una
dirección de pixel art retro. La clínica es un refugio deteriorado, con
una ventana tapiada, pintura descascarada y muebles abollados. La penumbra
azul carbón domina el cuarto. Una lámpara improvisada concentra la luz
crema en la mesa de examen. Afuera, la misión transcurre en un campo abierto.
Las dos escenas comparten la escala, los contornos y las sombras por bloques.
Cada una tiene sus propios colores de fondo.

Los dueños tienen retratos sobrios, con miradas cansadas, gestos contenidos
y ropa de trabajo remendada. Las mascotas comparten la penumbra y los
colores apagados. Se distinguen por las orejas asimétricas y la postura
de cada una. La radiación fantástica aparece en detalles pequeños,
como un frasco luminoso, el mechón de la cola de Mora y las manchas de Toby.

## Resolución y dibujo

- Componer en un lienzo interno de **640 × 360 px** (16:9). Es el espacio
  donde se dibujan los paneles y la parte visible del mapa. El tamaño de la
  ventana puede cambiar y el mapa puede extenderse más allá de la cámara.
- Escalar el lienzo por múltiplos enteros con filtro de punto. A 1280 × 720
  se muestra a ×2 y a 1920 × 1080 a ×3. Si la ventana es de 1366 × 768,
  usar ×2 y centrar con márgenes. Así los píxeles mantienen el mismo tamaño
  entre sí y la clínica conserva espacio para el diálogo y la ficha lateral.
  Para la demo, tomar 640 × 360 como tamaño mínimo de la ventana.
- Usar una cuadrícula de 16 px para las piezas del mapa. Mantener bordes
  definidos, sombras por bloques y dos o tres tonos por material. Reservar
  el mayor contraste para personajes, suministros e información del HUD.
- Exportar los sprites en PNG con transparencia y los fondos en PNG opaco.
  Mantener el mismo tamaño de fotograma y punto de apoyo en ambas
  direcciones.

## Paleta de la clínica

| Uso | Color |
|---|---|
| Contornos, panel inferior y sombras profundas | Carbón azulado `#161F26` |
| Paredes y fondos principales | Azul petróleo `#233C49` |
| Muebles y metal | Gris apagado `#526571` |
| Texto y luz de la clínica | Crema `#F3E9D2` |
| Detalles del refugio y acento de rechazo | Rojo ladrillo `#C65353` |
| Remiendos y prendas gastadas | Rojo ladrillo apagado `#784647` |
| Confirmación | Verde menta `#87C7A1` |
| Brillos de radiación fantástica | Verde lima `#B9CC57` |

El azul petróleo y el rojo ladrillo identifican la clínica. Los paneles usan
fondo carbón y texto crema. Marcar los botones con su rótulo y un borde
visible al seleccionarlos. Usar menta en la acción principal.

## Paleta del campo

| Uso | Color inicial |
|---|---|
| Pasto y cobertura baja | Verde oliva `#586346` |
| Claros y hierba seca | Ocre apagado `#8B7854` |
| Tierra y troncos | Marrón tierra `#695340` |
| Rocas y restos dispersos | Gris piedra `#83847A` |
| Sombras de árboles y fondo del HUD | Verde oscuro `#2B352C` |
| Suministros | Verde menta `#87C7A1` |
| Veterinario y texto del HUD | Crema `#F3E9D2` |

El pasto y la tierra usan tonos apagados. El veterinario se reconoce por su
silueta y un detalle crema. Los suministros se distinguen por forma y acento
menta. Los avisos de vida baja y tiempo agotado usan texto o iconos visibles.

## Tipografía

| Familia | Dónde se usa | Tamaño en el lienzo base |
|---|---|---|
| Pixelify Sans | Título del juego, encabezados como **Ficha de examen** y títulos del menú. | 24–32 px |
| Handjet | Diálogos, botones, etiquetas, cantidades y contador. Usar el trazo de matriz regular, con peso inicial 500–600. | 20–22 px en diálogos, 18–20 px en botones y HUD |

Pixelify Sans marca la jerarquía en encabezados cortos. Handjet da al texto
cotidiano el aspecto de un visor de matriz. Mantener la forma de sus puntos
estable y probar el peso en diálogo sobre fondo carbón. Probar en la ficha
de Mora esta muestra a tamaño real:
**«¿Qué necesita Mora? Vendas × 2 y antiséptico × 1.»**

Al incorporar las fuentes, guardar los TTF y sus archivos de licencia en
`arte/tipografias/`. Comprobar que la interfaz dibuje tildes, ñ, signos de
apertura y el símbolo ×. [Pixelify Sans](https://fonts.google.com/specimen/Pixelify+Sans) ·
[Licencia de Pixelify Sans](https://github.com/google/fonts/blob/main/ofl/pixelifysans/OFL.txt) ·
[Handjet y licencia](https://github.com/rosettatype/handjet)

## Composición de las pantallas

**Inicio.** Vista exterior nocturna del consultorio. El título **PET
SURVIVORS** queda sobre la zona oscura a la izquierda y debajo va el botón
ilustrado **Jugar**. La vista dibuja ambos PNG sobre el fondo y usa el
rectángulo del botón para detectar el clic. El cursor personalizado se
coloca con la punta de la flecha sobre la posición del ratón.

**Clínica.** Fondo fijo con estantes reutilizados, una mesa de examen y luz
crema sobre los personajes. El dueño y la mascota se ven en la parte
superior. El diálogo ocupa la cuarta parte inferior. Al pulsar **Examinar
mascota**, se abre la ficha de la derecha con los suministros, **Aceptar**
y **Rechazar**. El fondo se entrega sin texto para que la vista dibuje rótulos
y botones. [Boceto de la clínica](../src/vista/README.md#clínica)

**Misión.** Vista cenital, terreno de poco contraste y obstáculos con
contorno claro. Los animales tienen silueta baja. Los supervivientes
tienen silueta humana y el atacante a distancia una pose distinguible.
El HUD muestra vida y tiempo arriba y cantidades pendientes en una zona
que deje libre el centro. El menú de pausa conserva la forma de los paneles
y las fuentes de la clínica, con los colores de la escena activa.

## Mapa de la demo

El mapa es un campo cenital abierto. El pasto, la tierra y los claros se
componen con piezas de 16 × 16 px. El veterinario puede moverse en cualquier
dirección y elegir por dónde rodear la horda. Los árboles, las rocas y los
troncos aparecen de forma dispersa y dejan espacio para pasar entre ellos.

Para la primera versión, distribuir esos elementos al iniciar el intento y
revelarlos conforme se desplaza la cámara. Los suministros se colocan en
espacios libres. La segunda mascota reutiliza el conjunto de piezas del campo
y puede tener otra distribución de obstáculos y suministros.

Preparar primero pasto, tierra, una transición entre ambos y un árbol.
Colocar hierba baja y restos pequeños como decoración. Usar la base del
tronco como referencia de colisión y mantener libre el paso alrededor de la
copa. [Plan del mapa y oleadas](../src/modelo/mision/mapa/README.md)

## Archivos para la demo

| Recurso | Tamaño base | Ruta |
|---|---:|---|
| Fondo de inicio | 640 × 360 px | `arte/inicio/fondo.png` |
| Título ilustrado | 256 × 85 px | `arte/inicio/titulo.png` |
| Botón Jugar | 128 × 43 px | `arte/inicio/boton_jugar.png` |
| Cursor normal | 24 × 22 px | `arte/inicio/cursor_normal.png` |
| Fondo fijo de la clínica | 640 × 360 px | `arte/clinica/fondo.png` |
| Dueño, pose frontal o tres cuartos | 96 × 128 px | `arte/duenos/<id>.png` |
| Mascota | 64 × 64 px | `arte/mascotas/<id>.png` |
| Veterinario y supervivientes | 32 × 32 px por fotograma | `arte/veterinario/` y `arte/enemigos/<tipo>/` |
| Enemigo animal | 24 × 24 px por fotograma | `arte/enemigos/animal_de_contacto/` |
| Piezas de terreno y suministros | 16 × 16 px por pieza | `arte/mapa/` y `arte/suministros/<id>.png` |
| Árbol | 32 × 48 px | `arte/mapa/obstaculos/arbol.png` |

Cada personaje de misión tendrá una hoja `caminar_izquierda.png` y otra
`caminar_derecha.png`. Para la primera versión, usar cuatro fotogramas en una
fila, con los pies al mismo nivel. Indicar qué fotograma se usa en reposo.
Las fichas JSON señalan los PNG del dueño y de la mascota por su ruta desde
la raíz del proyecto. [Formato de fichas](../datos/README.md)

Los retratos alternativos disponibles son `elena_frente.png`,
`elena_mirada_baja.png`, `dueno_toby_frente.png` y `dueno_toby_mueca.png`.
Se guardan en `arte/duenos/` y tienen el mismo tamaño que los retratos base.

## Prompts de la pantalla inicial

| Recurso | Prompt | PNG final |
|---|---|---|
| Fondo exterior | [Calle y fachada de noche](prompts/inicio/fondo_inicio.txt) | `arte/inicio/fondo.png` |
| Título ilustrado | [PET SURVIVORS](prompts/inicio/titulo_pet_survivors.txt) | `arte/inicio/titulo.png` |

Generar primero el fondo. Adjuntarlo al pedir el título para comprobar
el contraste. Dar al título unos 256 px de ancho en el lienzo de 640 × 360 px.

## Prompts de la clínica

| Recurso | Prompt | PNG final |
|---|---|---|
| Consultorio | [Fondo de la clínica](prompts/clinica/escenarios/fondo_clinica.txt) | `arte/clinica/fondo.png` |
| Elena | [Dueña de Mora](prompts/clinica/personajes/elena.txt) | `arte/duenos/elena.png` |
| Mora | [Gata de orejas asimétricas y cola luminosa](prompts/clinica/personajes/mora.txt) | `arte/mascotas/mora.png` |
| Dueño de Toby | [Dueño con chaqueta de trabajo y bufanda gastada](prompts/clinica/personajes/dueno_toby.txt) | `arte/duenos/dueno_toby.png` |
| Toby | [Perro mestizo cansado con manchas luminosas y manta](prompts/clinica/personajes/toby.txt) | `arte/mascotas/toby.png` |

Generar primero el fondo y usarlo como referencia al generar por separado
a Elena y Mora. Comprobar que el tamaño y la iluminación encajen antes de
producir a Toby y a su dueño. Cada personaje se entrega como PNG transparente.

### Variantes para el diálogo de tratamiento

Generar un sprite por archivo. Adjuntar el sprite base aceptado y aplicar
el prompt de edición correspondiente, para conservar identidad, escala y
punto de apoyo al cambiar la expresión. Los PNG base son los que se usan
en el JSON de las fichas. Las variantes quedan listas para cuando la vista
seleccione el sprite según la etapa del diálogo.

| Personaje | Prompt de edición | PNG final |
|---|---|---|
| Elena | [Elena aliviada](prompts/clinica/personajes/variantes/elena_aliviada.txt) | `arte/duenos/elena_aliviada.png` |
| Mora | [Mora relajada](prompts/clinica/personajes/variantes/mora_relajada.txt) | `arte/mascotas/mora_relajada.png` |
| Dueño de Toby | [Dueño de Toby aliviado](prompts/clinica/personajes/variantes/dueno_toby_aliviado.txt) | `arte/duenos/dueno_toby_aliviado.png` |
| Toby | [Toby recuperado](prompts/clinica/personajes/variantes/toby_recuperado.txt) | `arte/mascotas/toby_recuperado.png` |

## Entrega del arte

1. Preparar una prueba de estilo con el fondo de la clínica, Mora, su dueña,
   el veterinario, un animal, las vendas y una muestra del campo con un árbol.
   Revisarla con el rol 5 a tamaño real antes de producir el resto de las hojas.
2. Cuando la prueba encaje, completar los dos casos, las direcciones de
   caminata, los supervivientes y los suministros de la demo. El rol 3
   entrega los PNG aprobados, sus rutas y la cuadrícula de cada hoja.
3. Para generar variantes con ComfyUI en Windows, el comando previsto sigue
   siendo `py arte/generar_sprites.py`. El flujo leerá la lista de recursos
   y dejará variantes para que el equipo elija los PNG finales.
