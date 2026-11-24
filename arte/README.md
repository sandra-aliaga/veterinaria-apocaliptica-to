# Arte

Guardar aquí los PNG aprobados. Las fichas identifican el retrato de la mascota
y el sprite del dueño por la ruta relativa a la raíz del proyecto. Reservar
`mascotas/` y `duenos/` para esos recursos.

La interfaz puede comenzar con figuras provisionales. El equipo de arte puede
producir los PNG en Windows y comprobar que se distingan al tamaño de juego.

## Paleta para la primera prueba

Usar azul petróleo para las sombras y el terreno, rojo ladrillo para los
enemigos y avisos de daño, y crema claro para el texto y los bordes del
veterinario. Los suministros llevan un acento verde menta y una silueta propia.
Así se pueden ubicar aunque haya muchos enemigos alrededor.

| Uso | Color inicial |
|---|---|
| Sombras y terreno | Azul petróleo `#233C49` |
| Enemigos y daño | Rojo ladrillo `#C65353` |
| Texto y contrastes | Crema `#F3E9D2` |
| Suministros y recogida | Verde menta `#87C7A1` |

Probar juntos el fondo, el veterinario y una horda a escala de juego antes
de cerrar la paleta. La forma y el brillo también ayudan a distinguir los
objetos y personajes.

La clínica es un refugio improvisado después del colapso. El fondo puede
mostrar muebles reparados, estantes reutilizados y una mesa de examen. En la
misión, los animales tienen una silueta baja y los supervivientes una silueta
humana. Dar al atacante a distancia una pose reconocible entre la horda.

## Veterinario y enemigos

Preparar `caminar_izquierda.png` y `caminar_derecha.png` por personaje. Guardar
las hojas del jugador en `veterinario/` y las de los enemigos en `enemigos/`,
con una subcarpeta por tipo: `animal_de_contacto/`, `superviviente_de_contacto/`
y `superviviente_a_distancia/`. Usar la misma cuadrícula y el mismo punto de apoyo
en ambos lados para que la figura no salte al girar. Para el reposo se puede
usar un fotograma de la hoja del lado correspondiente.
