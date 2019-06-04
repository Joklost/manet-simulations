from typing import List
import math

THERMAL_NOISE = -119.66
NOISE_FIGURE = 4.2


def lin(log_):
    return math.pow(10, log_ / 10)


def log(lin_):
    return 10 * math.log10(lin_)


def pepe(rssi: float, packetsize: int, interference: List[float]):
    p_n = lin(THERMAL_NOISE + NOISE_FIGURE)

    p_i = 0.0
    for rssi_i_db in interference:
        p_i += lin(rssi_i_db)

    p_ni = p_n + p_i
    p_ni_db = log(p_ni)
    snir_db = rssi - p_ni_db
    snir = lin(snir_db)

    bep = 0.5 * math.erfc(math.sqrt(snir / 2.0))
    pep = 1.0 - math.pow(1.0 - bep, float(packetsize) * 8.0)

    return pep


def main():
    min = -112
    max = -102
    step = 0.1

    curr = min
    print('\\addplot[very thick, solid, cyan!50!black] coordinates {', end='')
    while curr < max:
        rssi = round(curr, 1)
        pep = pepe(rssi, 20, [])
        print(f'({rssi},{pep})', end='')
        curr += step

    print('};')

    min = -70
    max = -60
    step = 0.1

    curr = min
    print('\\addplot[very thick, solid, cyan!50!black] coordinates {', end='')
    while curr < max:
        rssi = round(curr, 1)
        pep = pepe(rssi, 20, [-74.042])
        print(f'({rssi},{pep})', end='')
        curr += step

    print('};', end='')


if __name__ == '__main__':
    main()