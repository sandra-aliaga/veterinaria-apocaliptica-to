"""Comprueba o aplica el formato C++ del repositorio."""

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
EXTENSIONES = ("*.h", "*.hpp", "*.c", "*.cc", "*.cpp", "*.cxx")
TEXTO_PROTEGIDO = re.compile(
    rb"^[ \t]*\#(?:[^\r\n]*\\\r?\n)*[^\r\n]*"
    rb"|//(?:[^\r\n]*\\\r?\n)*[^\r\n]*|/\*.*?\*/"
    rb'|R"(?P<delimitador>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimitador)"'
    rb'|"(?:\\(?:\r\n|.)|[^"\\\r\n])*"'
    rb"|'(?:\\(?:\r\n|.)|[^'\\\r\n])*'",
    re.DOTALL | re.MULTILINE,
)
BLOQUES_JUNTOS = re.compile(
    rb"^([ \t]*)}[ \t]*(\r?\n)\1(?=(?:if|else|for|while|do|switch|try|catch)\b)",
    re.MULTILINE,
)


def separar_bloques(codigo):
    """Inserta una línea vacía entre bloques consecutivos del mismo nivel."""
    # Conserva las posiciones y los finales de linea mientras oculta texto literal.
    codigo_visible = TEXTO_PROTEGIDO.sub(
        lambda coinc: re.sub(rb"[^\r\n]", b" ", coinc[0]), codigo
    )
    partes = []
    inicio = 0

    for bloque in BLOQUES_JUNTOS.finditer(codigo_visible):
        salto = bloque.end(2)
        partes.extend((codigo[inicio:salto], bloque[2]))
        inicio = salto

    partes.append(codigo[inicio:])
    return b"".join(partes)


def formatear(ruta, contenido):
    """Aplica clang-format y la separación de bloques del proyecto."""
    resultado = subprocess.run(
        ["clang-format", "--Werror", f"--assume-filename={ruta}"],
        input=contenido,
        stdout=subprocess.PIPE,
        check=True,
        cwd=RAIZ,
    )
    return separar_bloques(resultado.stdout)


def archivos_cpp(preparados):
    """Enumera fuentes locales o fuentes en el índice de Git."""
    if preparados:
        opciones = ["diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z"]
    else:
        opciones = ["ls-files", "--cached", "--others", "--exclude-standard", "-z"]

    rutas = subprocess.check_output(["git", *opciones, "--", *EXTENSIONES], cwd=RAIZ)
    return sorted({os.fsdecode(ruta) for ruta in rutas.split(b"\0") if ruta})


def main():
    """Comprueba fuentes o corrige archivos de trabajo explícitamente."""
    argumentos = argparse.ArgumentParser(description=__doc__)
    modos = argumentos.add_mutually_exclusive_group()
    modos.add_argument(
        "--corregir", action="store_true", help="Escribe el formato en los archivos."
    )
    modos.add_argument(
        "--preparados", action="store_true", help="Comprueba el contenido del índice."
    )
    argumentos.add_argument(
        "archivos", nargs="*", help="Rutas relativas a la raíz del repositorio."
    )
    opciones = argumentos.parse_args()

    if opciones.preparados and opciones.archivos:
        argumentos.error("--preparados ya toma los archivos del índice de Git.")

    resultado = 0
    rutas = opciones.archivos or archivos_cpp(opciones.preparados)

    for nombre in rutas:
        ruta = RAIZ / nombre

        if opciones.preparados:
            original = subprocess.check_output(["git", "show", f":{nombre}"], cwd=RAIZ)
        elif ruta.is_file():
            original = ruta.read_bytes()
        elif opciones.archivos:
            argumentos.error(f"No existe el archivo {nombre}.")
        else:
            continue

        esperado = formatear(ruta, original)

        if original == esperado:
            continue

        if opciones.corregir:
            ruta.write_bytes(esperado)
        else:
            print(
                f"Formato incorrecto en {nombre}. "
                "Ejecuta python pruebas/formato_cpp.py --corregir.",
                file=sys.stderr,
            )
            resultado = 1

    return resultado


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError) as error:
        sys.exit(f"No se pudo comprobar el formato C++: {error}")
