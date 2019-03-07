import subprocess
import time

NOTHING_RECEIVED = 'Nothing received!'
PACKET_DROPPED = 'packet_dropped'

coordr = r'/home/joklost/repos/Masters/src/cmake-build-debug/coordr/coordr'
program = r'/home/joklost/repos/Masters/src/cmake-build-debug/scratch/scratch'
lmc = r'/home/joklost/repos/Masters/src/cmake-build-debug/lmc/lmc'

nodes = 250

start = time.time()
command = ['mpirun', '-n', '1', coordr, ':', '-n', nodes, program, ':', '-n', '1', lmc]
output = subprocess.check_output(command).decode().split('\n')
end = time.time()

dropped_packages = 0
nothing_received = 0

for line in output:
    if NOTHING_RECEIVED in line:
        nothing_received += 1
    elif PACKET_DROPPED in line:
        dropped_packages += 1

print('========================')
print('nodes', nodes)
print('took', end - start)
print('dropped_packages', dropped_packages)
print('nothing_received', nothing_received)