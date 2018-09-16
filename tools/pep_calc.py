import math

packet_size = 160  # bits
print('packet\t', packet_size)
rssi = -105.3  # dBm
print('rssi\t', rssi)
noise_power = -119.66 + 4.2  # thermal_noise - noise_figure
print('noise\t', noise_power)
y_db = rssi - noise_power  # signal to noise ratio in decibel
print('y_db\t', y_db)
y = math.pow(10, y_db / 10)  # signal to noise ratio in power ratio
print('y\t', y)
bep = (1 / 2) * math.erfc(math.sqrt(y / 2))  # bit error probabiltiy
print('bep\t', bep)
pep = 1 - math.pow((1 - bep), packet_size)  # packet error probability
print('pep\t', pep)
