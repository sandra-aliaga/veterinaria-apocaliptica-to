# Prueba jugable

Esta versión permite recorrer la portada, la clínica y la misión con las
fichas de ejemplo de Mora y Toby.

[`configuracion_prueba.h`](configuracion_prueba.h) reúne el tamaño del campo,
los obstáculos, la duración de la misión y los ajustes de movimiento.

[`ControladorPrueba`](controlador_prueba.h) implementa el contrato
[`Controlador`](../controlador/controlador.h). [`main.cpp`](../main.cpp)
crea el controlador y conecta sus acciones y su estado de pantalla con la
vista. Al integrar el juego, el arranque puede conectar otro controlador
que cumpla el mismo contrato.

[`guardado_partida_prueba.cpp`](guardado_partida_prueba.cpp) implementa los
métodos que convierten el estado a texto y lo reconstruyen al cargar.
[`ArchivoPartida`](../persistencia/archivo_partida.h) recibe la ruta y
el texto para gestionar el archivo. Esta versión escribe la partida en
`datos/progreso_previa.txt`.

Las [verificaciones automáticas](../../pruebas/verificar_base.py) están en
`pruebas/`.
