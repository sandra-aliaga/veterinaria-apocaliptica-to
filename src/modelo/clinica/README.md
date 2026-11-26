# Rol 1 · Consultorio y cola

Implementar las reglas de atención y la cola enlazada. El [rol 5](../../controlador/README.md)
se encarga de la pantalla, del examen y de los diálogos. Puedes probar el
consultorio en consola antes de que exista la ventana.

## Entrega

- [`ColaCasosClinicos`](cola_casos.h) crea y libera sus nodos. Cada nodo contiene
  una ficha y un enlace propietario al siguiente. `primero_` posee el primer
  nodo. `ultimo_` es prestado, permite encolar en tiempo constante y queda
  nulo al extraer el último. `frente()` consulta sin retirar.
- [`Consultorio`](consultorio.h) recibe las fichas en orden y mantiene el caso
  activo y el saldo. Las consultas de ficha pendiente y activa devuelven
  punteros prestados que se usan antes de modificar el consultorio.
- Al aceptar, extraer la primera ficha, conservarla como activa y devolver
  una `PropuestaMision` con su ID y todos los suministros requeridos. Al
  rechazar, retirar la primera ficha y seguir con la próxima. Si ya hay un
  caso activo o la cola está vacía, esas acciones carecen de efecto.
- Registrar el resultado del intento según la tabla. El reintento se habilita
  después de un fallo registrado y devuelve el objetivo original. Solo puede
  haber un intento activo a la vez.

| Resultado recibido | Respuesta de `registrar_resultado()` |
|---|---|
| El consultorio no tiene un caso activo. | `sin_caso_activo`. |
| El ID no coincide, ya se registró el intento o un supuesto éxito no cubre todas las cantidades. | `resultado_invalido`. Se conserva el caso y el saldo. |
| El intento activo termina por falta de vida o tiempo. | `fallo_registrado`. Se conserva el caso y su recompensa pendiente. |
| El intento activo reúne las cantidades requeridas. | `tratado`. Se suma la recompensa una vez y se cierra el caso. |

## Prueba independiente

1. Encola dos fichas, comprueba el orden y vuelve a insertar después de
   vaciar la cola.
2. Acepta la primera. Envía un éxito con un suministro faltante y comprueba
   que siga activa y que el saldo no suba.
3. Registra un fallo válido, reintenta con los mismos requisitos y envía
   un resultado completo. Comprueba que pague una vez aunque se entregue el
   mismo resultado de nuevo.
4. Rechaza otra ficha y comprueba que la cola avance sin iniciar una misión.

## Implementación y pruebas automáticas

- [`ConsultorioClinica`](consultorio_clinica.h) implementa la interfaz y
  [`ColaCasosClinicos`](cola_casos.cpp) la cola. Un fallo debe traer
  `motivo_fallo`; si falta, el resultado es inválido.
- [`pruebas_consultorio.cpp`](../../../pruebas/pruebas_consultorio.cpp) cubre los
  cuatro pasos de la prueba independiente sin ventana ni dependencias nuevas.
  Desde la raíz del repositorio:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -o build/pruebas_consultorio \
    pruebas/pruebas_consultorio.cpp src/modelo/clinica/*.cpp
./build/pruebas_consultorio
```

El programa termina con código 0 si todo pasa y con 1 si falla alguna comprobación.
