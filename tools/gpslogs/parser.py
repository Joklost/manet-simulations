def safe_pop(iterator):
    if iterator:
        return iterator.pop(0)

    return None


def safe_float(string):
    if string:
        return float(string)

    return None


def safe_int(string):
    if string:
        return int(string)

    return None


def safe_hex(string):
    if string:
        return int(string, 16)

    return None


def main():
    files = ['rude_skov.csv', 'bagongsilangan.csv', 'marikina.csv', 'phillippines.csv', 'rizal.csv', 'sep_drill.csv']
    for file in files:
        with open(file) as csv:
            lines = csv.readlines()

        gpslog = []
        for line in lines[1:]:
            it = line.strip().split(';')
            time = safe_int(safe_pop(it)) * 1000  # convert to ms
            id_ = safe_hex(safe_pop(it))
            safe_pop(it)
            lat = safe_float(safe_pop(it))
            lon = safe_float(safe_pop(it))

            if not id_ or not time or not lat or not lon:
                continue

            neigbours = []
            while it:
                n_id = safe_int(safe_pop(it))
                n_rssi = safe_int(safe_pop(it))
                safe_pop(it)

                if not n_id or not n_rssi:
                    continue
                neigbours.append((n_id, n_rssi))

            gpslog.append({'id': id_, 'lat': lat, 'lon': lon, 'time': time, 'neighbours': neigbours})

        mintime = min([log["time"] for log in gpslog])
        name = file.split('.')[0]
        with open(f"{name}_gps.txt", 'w') as gps, open(f'{name}_gps_rssi.txt', 'w') as gps_rssi:
            for line in gpslog:
                gps.write(f'{line["id"]},{line["lat"]:f},{line["lon"]:f},{line["time"] - mintime:f}')
                gps_rssi.write(f'{line["id"]},{line["lat"]:f},{line["lon"]:f},{line["time"] - mintime:f}')

                for id_, rssi in line["neighbours"]:
                    gps_rssi.write(f',{id_},{rssi}')

                gps.write('\n')
                gps_rssi.write('\n')


if __name__ == '__main__':
    main()
