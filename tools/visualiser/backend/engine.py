import glob
import math

# validate data and proxy to real functions
def execute(data, files):
    models = list_models()['models']
    if 'type' not in data:
        return {'error':'No type recieved'}
    if 'model' not in data:
        return {'error': 'No Model received'}
    if data['model'] not in models:
        return {'error': 'Model not available: ' + data['model'] + " use one of " + str(models)}
    if data['type'] == 'static':
        if 'topology' in data:
            if data['topology'] == 'grid':
                if 'number_of_nodes' in data and 'node_init_time' in data and 'duration' in data:
                    nn = 0
                    it = 0
                    dur = 0
                    try:
                        nn = int(data['number_of_nodes'])
                    except Exception:
                        {'error': "number_of_nodes is not a number"}
                    try:
                        it = int(data['node_init_time'])
                    except Exception:
                        {'error': "node_init_time is not a number"}
                    try:
                        dur = int(data['duration'])
                    except Exception:
                        {'error': "duration is not a number"}
                    return run_static_grid(nn, it, dur)
                else:
                    return {'error': 'Missing arguments for simulation'}
            else:
                return {'error': 'Unknown topology'}
        else:
            return {'error': "No topology received"}
    if data['type'] == 'gps':
        if 'gps_data' not in files:
            return {'error': "No GPS-log"}
        raw = files['gps_data'].read().decode().splitlines()
        parsed = []
        for i in range(len(raw)):

            entry = raw[i].split(",")
            if len(entry) < 4:
                return file_error(i, data['gps_data'].filename, 'less than four entries')
            id = 0
            lat = 0
            lon = 0
            ts = 0
            try:
                id = int(entry[0])
            except Exception:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not an id')
            if id < 0:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not an id')
            try:
                lat = float(entry[1])
            except Exception:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not a latitude')
            if lat < -90 or lat > 90:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not a latitude')
            try:
                lon = float(entry[2])
            except Exception:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not a latitude')
            if lon < -180 or lon > 180:
                return file_error(i, data['gps_data'].filename, 'entry 0 is not a latitude')
            try:
                ts = float(entry[3])
            except Exception:
                return file_error(i, data['gps_data'].filename, 'entry 3 is not a timestamp')
            if ts < 0:
                return file_error(i, data['gps_data'].filename, 'entry 3 is not a timestamp')
            parsed.append((id, lat, lon, ts))
        fdur = 0
        tdur = -1
        if 'from_duration' in data and len(data['from_duration'].strip()) > 0:
            try:
                fdur = int(data['from_duration'])
            except Exception:
                return {'error': "from_duration is not a number"}
        if 'to_duration' in data and len(data['to_duration'].strip()) > 0:
            try:
                tdur = int(data['to_duration'])
            except Exception:
                return {'error': "to_duration is not a number"}
        return run_gps(fdur, tdur, parsed)

    return {'error': "Unknown type or topology"}

def file_error(filename, line, message):
    return {'error': 'Line ' + str(line) + ' of \"' + filename + '\" - ' + message}

def list_models():
    res = []
    folder = "backend/models/"
    postfix = ".xml"
    for file in glob.glob(folder + "/*" + postfix):
        model = str(file)
        res.append(model[len(folder): - len(postfix)])
    return {"models": res}

def run_static_grid(num_nodes, init_time, duration):
    if num_nodes <= 0:
        return {'error': 'Expected at least one node'}
    if num_nodes >= 10000:
        return {'error': 'Expected less than 10000 nodes'}

    if init_time < 0:
        return {'error': 'Expected at least some init time'}
    if init_time >= duration:
        return {'error': 'Expected duration to be larger than init_time'}

    return {}

def run_gps(fdur, tdur, data):
    if tdur is not -1 and tdur <= fdur:
        return {'error': 'From duration should be smaller than to duration (' + str(fdur) + ", " + str(tdur) + ")"}

    nodes = {}
    minlat = math.inf
    maxlat = -math.inf
    minlon = math.inf
    maxlon = -math.inf
    mints = math.inf
    maxts = -math.inf
    for row in data:
        data = {'lat' : row[1], 'lon': row[2], 'timestamp': row[3]}
        if row[3] < fdur or (tdur is not -1 and row[3] > tdur):
            continue
        if row[0] not in nodes:
            nodes[row[0]] = [data]
        else:
            nodes[row[0]].append(data)
        minlat = min(minlat, row[1])
        maxlat = max(maxlat, row[1])

        minlon = min(minlon, row[2])
        maxlon = max(maxlon, row[2])

        mints = min(mints, row[3])
        maxts = max(maxts, row[3])
    if len(nodes) == 0:
        return {'error': 'No nodes within the duration (or file is empty or could not be parsed)'}
    for key in nodes:
        nodes[key].sort(key=lambda el: el['timestamp'])
    return {'nodes': nodes, 'min_lat': minlat, 'max_lat': maxlat, 'min_lon': minlon, 'max_lon': maxlon, 'first_time': mints, 'last_time': maxts}