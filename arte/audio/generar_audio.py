"""Genera el clic y la musica original de Pet Survivors como WAV mono."""

from array import array
from math import pi, sin, tau
from pathlib import Path
from random import Random
from sys import byteorder
import wave


MUESTRAS_POR_SEGUNDO = 22_050
PULSOS_POR_MINUTO = 90
MUESTRAS_POR_PULSO = MUESTRAS_POR_SEGUNDO * 60 // PULSOS_POR_MINUTO
PULSOS_POR_COMPAS = 4
DESTINO = Path(__file__).resolve().parent

# Re menor. La nota grave y el motivo se reparten ocho compases.
BAJOS = (38, 38, 34, 36, 38, 38, 34, 33)
MELODIA = (
    ((0.5, 62), (1.5, 65), (3, 69)),
    ((0.5, 67), (2, 64)),
    ((1, 65), (2.5, 62)),
    ((0.5, 60), (1.5, 64), (3, 67)),
    ((0.5, 62), (1.5, 65), (2.5, 69)),
    ((1, 72), (2.5, 69)),
    ((0.5, 65), (2, 62)),
    ((1, 64), (2, 61), (3, 57)),
)

# Un pulso del 25 % con armonicos limitados deja espacio al dialogo.
CICLO_PULSO = 0.25
ARMONICOS_PULSO = tuple(
    sin(pi * armonico * CICLO_PULSO) / armonico for armonico in range(1, 7)
)


def frecuencia(nota):
    return 440.0 * 2 ** ((nota - 69) / 12)


def pulso(fase):
    return sum(
        amplitud * sin(orden * fase)
        for orden, amplitud in enumerate(ARMONICOS_PULSO, start=1)
    )


def triangulo(fase):
    avance = (fase / tau) % 1.0
    return 1.0 - 4.0 * abs(avance - 0.5)


def sumar_nota(pista, inicio, duracion, nota, volumen, oscilador):
    muestra_inicial = round(inicio * MUESTRAS_POR_PULSO)
    cantidad = round(duracion * MUESTRAS_POR_PULSO)
    avance_fase = tau * frecuencia(nota) / MUESTRAS_POR_SEGUNDO
    ataque = round(0.008 * MUESTRAS_POR_SEGUNDO)
    salida = round(0.055 * MUESTRAS_POR_SEGUNDO)

    for i in range(cantidad):
        nivel = min(1.0, i / ataque, (cantidad - i - 1) / salida)
        indice = (muestra_inicial + i) % len(pista)
        pista[indice] += volumen * nivel * oscilador(i * avance_fase)


def crear_musica():
    pista = [0.0] * (len(BAJOS) * PULSOS_POR_COMPAS * MUESTRAS_POR_PULSO)

    for compas, (bajo, motivo) in enumerate(zip(BAJOS, MELODIA)):
        inicio = compas * PULSOS_POR_COMPAS

        for desplazamiento, nota in ((0, bajo), (1.5, bajo + 12), (2.5, bajo + 7)):
            sumar_nota(pista, inicio + desplazamiento, 0.85, nota, 0.14, triangulo)

        for desplazamiento, nota in motivo:
            sumar_nota(pista, inicio + desplazamiento, 0.60, nota, 0.18, pulso)
            sumar_nota(pista, inicio + desplazamiento + 0.50, 0.45, nota, 0.035, pulso)

    return pista


def crear_clic():
    generador = Random(208)
    cantidad = round(0.06 * MUESTRAS_POR_SEGUNDO)
    fase = 0.0
    muestras = []

    for i in range(cantidad):
        progreso = i / cantidad
        fase += tau * (820.0 - 330.0 * progreso) / MUESTRAS_POR_SEGUNDO
        entrada = min(1.0, i / 22)
        volumen = entrada * (1.0 - progreso) ** 3
        muestras.append(volumen * (0.39 * pulso(fase) + 0.055 * generador.uniform(-1, 1)))

    muestras[-1] = 0.0
    return muestras


def guardar_wav(archivo, muestras):
    pcm = array("h", (round(max(-1.0, min(1.0, valor)) * 32767) for valor in muestras))
    if byteorder == "big":
        pcm.byteswap()

    with wave.open(str(DESTINO / archivo), "wb") as salida:
        salida.setparams((1, 2, MUESTRAS_POR_SEGUNDO, 0, "NONE", "not compressed"))
        salida.writeframes(pcm.tobytes())


if __name__ == "__main__":
    guardar_wav("boton.wav", crear_clic())
    guardar_wav("refugio.wav", crear_musica())
