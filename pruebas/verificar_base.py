"""Comprueba las cabeceras y los datos del proyecto sin abrir una ventana."""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

RAIZ = Path(__file__).resolve().parent.parent


def verificar_cabeceras(compilador):
    cabeceras = sorted(
        cabecera
        for directorio in ("include", "src")
        for cabecera in (RAIZ / directorio).rglob("*.h")
    )
    if not cabeceras:
        raise ValueError("No se encontraron cabeceras de C++.")

    opciones = [
        compilador,
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Werror",
        "-Iinclude",
        "-Isrc",
        "-x",
        "c++",
        "-fsyntax-only",
        "-",
    ]
    for cabecera in cabeceras:
        relativa = cabecera.relative_to(RAIZ).as_posix()
        subprocess.run(
            opciones,
            cwd=RAIZ,
            input=f'#include "{relativa}"\n',
            text=True,
            check=True,
        )
    print(f"Cabeceras C++17 verificadas: {len(cabeceras)}.")


def verificar_ficha(ficha, origen):
    if (
        not isinstance(ficha, dict)
        or not isinstance(ficha.get("id"), str)
        or not ficha["id"].strip()
    ):
        raise ValueError(f"{origen}: se requiere un identificador de caso.")
    suministros = ficha.get("suministros")
    if not isinstance(suministros, list) or not suministros:
        raise ValueError(f"{origen}: se requiere al menos un suministro.")

    identificadores = set()
    for suministro in suministros:
        if not isinstance(suministro, dict):
            raise TypeError(f"{origen}: cada suministro debe ser un objeto.")
        identificador = suministro.get("id")
        cantidad = suministro.get("cantidad")
        if not isinstance(identificador, str) or not identificador.strip():
            raise ValueError(f"{origen}: cada suministro requiere un identificador.")
        if identificador in identificadores:
            raise ValueError(f"{origen}: suministro repetido: {identificador}.")
        if type(cantidad) is not int or cantidad <= 0:
            raise ValueError(
                f"{origen}: {identificador} requiere una cantidad entera positiva."
            )
        identificadores.add(identificador)


def verificar_datos():
    directorio = RAIZ / "datos"
    guia = (directorio / "README.md").read_text(encoding="utf-8")
    ejemplo = re.search(r"```json\s*\n(.*?)\n```", guia, flags=re.DOTALL)
    if ejemplo is None:
        raise ValueError("Falta el JSON de ejemplo en datos/README.md.")
    verificar_ficha(json.loads(ejemplo[1]), "datos/README.md")

    casos = set()
    for archivo in sorted(directorio.glob("*.json")):
        ficha = json.loads(archivo.read_text(encoding="utf-8"))
        verificar_ficha(ficha, archivo.name)
        if ficha["id"] in casos:
            raise ValueError(
                f"{archivo.name}: identificador de caso repetido: {ficha['id']}."
            )
        casos.add(ficha["id"])
    print(f"Datos verificados: un ejemplo y {len(casos)} fichas JSON.")


def verificar_documentacion():
    documentos = [RAIZ / "README.md"]
    for directorio in ("include", "src", "datos", "arte", "docs"):
        documentos.extend((RAIZ / directorio).rglob("README.md"))
    for documento in documentos:
        texto = documento.read_text(encoding="utf-8")
        for destino in re.findall(r"\]\(([^)]+)\)", texto):
            enlace = urlsplit(destino)
            if enlace.scheme or enlace.netloc or not enlace.path:
                continue
            if not (documento.parent / unquote(enlace.path)).exists():
                relativa = documento.relative_to(RAIZ).as_posix()
                raise ValueError(f"{relativa}: enlace local ausente: {destino}.")
    print(f"README verificados: {len(documentos)}.")


def main():
    argumentos = argparse.ArgumentParser(description=__doc__)
    argumentos.add_argument(
        "--compilador", default="g++", help="Ejecutable de GCC o Clang."
    )
    opciones = argumentos.parse_args()
    try:
        verificar_cabeceras(opciones.compilador)
        verificar_datos()
        verificar_documentacion()
    except (OSError, TypeError, ValueError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
