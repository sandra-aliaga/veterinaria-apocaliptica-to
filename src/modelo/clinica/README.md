# Consultorio

Este frente implementa la cola enlazada y el consultorio. Puede probar ambos
con fichas construidas directamente en C++.

- La cola posee el primer nodo y cada nodo posee el siguiente. El puntero al
  último nodo sirve para insertar al final y queda nulo al vaciar la cola.
  Consultar el frente no retira la ficha.
- Al aceptar, el consultorio extrae la primera ficha y conserva el caso activo.
  La propuesta incluye una copia de toda la lista de suministros requeridos.
  Al rechazar, descarta la primera ficha sin iniciar una misión.
- Al recibir un resultado, verifica el identificador del caso y las cantidades
  recogidas por suministro. Si el intento falla, conserva el caso para
  reintentar. Si completa el tratamiento, cierra el caso y cobra una sola vez.
- Un nuevo intento solo se habilita después de registrar el fallo. Durante un
  intento activo se conserva la ficha y el flujo de atención de ese caso.

Probar la cola vacía, extraer dos fichas en orden, volver a insertar tras
vaciarla y simular un resultado que tenga solo parte de los suministros.
