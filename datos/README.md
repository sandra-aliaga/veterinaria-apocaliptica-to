# Fichas de casos

Guardar un archivo JSON por caso en este directorio. El catálogo recorrerá los
archivos por nombre para tener un orden de llegada reproducible. Por ejemplo,
`01_mora.json` y `02_toby.json`. Este bloque ilustra una ficha completa:

```json
{
  "id": "caso_mora",
  "mascota": {
    "id": "mora",
    "nombre": "Mora",
    "especie": "gato",
    "edad_meses": 24,
    "retrato": "arte/mascotas/mora.png"
  },
  "dueno": {
    "id": "elena",
    "nombre": "Elena",
    "sprite": "arte/duenos/elena.png"
  },
  "dialogos": {
    "llegada": [
      { "hablante": "dueno", "texto": "Hola, traje a Mora porque se lastimó una pata." },
      { "hablante": "veterinario", "texto": "Voy a revisar qué necesitamos." },
      { "hablante": "dueno", "texto": "Se queda aquí conmigo." }
    ],
    "aceptacion": [
      { "hablante": "veterinario", "texto": "Iré por los suministros." }
    ],
    "rechazo": [],
    "fallo": [
      { "hablante": "dueno", "texto": "Mora sigue aquí. Puedes intentarlo otra vez." }
    ],
    "tratamiento": [
      { "hablante": "veterinario", "texto": "Ya pude curarle la pata." },
      { "hablante": "dueno", "texto": "Gracias por atender a Mora." }
    ]
  },
  "suministros": [
    { "id": "vendas", "cantidad": 2 },
    { "id": "antiseptico", "cantidad": 1 }
  ],
  "recompensa": 15
}
```

- **Diálogos.** Cada etapa acepta una lista en el orden en que se muestra.
  Una lista vacía se salta. Cada línea indica el hablante y su texto. Las
  etapas son llegada, aceptación, rechazo, fallo y tratamiento.
- **Suministros.** Cada ficha necesita al menos un tipo. Se escribe una entrada
  por identificador, con una cantidad entera positiva. Para necesitar solo
  vendas se deja una entrada. Para necesitar más, se cambia su cantidad o
  se agregan otros identificadores.
- **Mascota.** Usar `gato` o `perro` y una edad entera en meses mayor o igual a cero.
  La ficha muestra la edad equivalente que calcula el tipo de mascota.
- **Identificadores.** Los de caso deben ser únicos. Los de mascota y dueño
  identifican a esos personajes dentro de las fichas. El mapa y el consultorio
  utilizan el mismo identificador para cada suministro.
- **Recursos.** Las rutas se leen desde la raíz de `proyecto_final/`, con
  separadores `/`. Guardar el texto en UTF-8 y la recompensa como un entero
  mayor o igual a cero.

Para añadir otro caso con una mascota y un dueño nuevos, agregar su ficha y los
PNG correspondientes. Si la ficha tiene errores, el catálogo informa qué
archivo y qué dato hay que corregir.
