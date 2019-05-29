import glob
from subprocess import Popen, PIPE, STDOUT
import os
from backend.smc2py import parseEngineStdout
import math
import copy


# validate data and proxy to real functions
def execute(data):
    models = list_models()['models']
    if 'type' not in data:
        return {'error': 'No type recieved'}
    if data['type'] not in ['log', 'log+rssi']:
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
                    return run_static_grid(data['model'], nn, it, dur)
                else:
                    return {'error': 'Missing arguments for simulation'}
            else:
                return {'error': 'Unknown topology'}
        else:
            return {'error': "No topology received"}

    error, parsed, edges = None, None, None
    if data['type'] in ['gps', 'log', 'log+rssi']:
        if 'gps_data' not in data:
            return {'error': "No GPS-log"}

        if data['type'] == 'log+rssi':
            error, parsed, edges = parse_gps(data['gps_data'], with_rssi=True)
        else:
            error, parsed, edges = parse_gps(data['gps_data'])

        if error is not None:
            return error

    if data['type'] in ['log', 'log+rssi']:
        return run_log(0, -1, parsed, edges)

    if data['type'] == 'gps':
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


def parse_gps(data, with_rssi: bool = False):
    raw = data.splitlines()
    parsed = []
    edges = {} if with_rssi else None
    for i in range(len(raw)):
        entry = raw[i].split(",")
        if len(entry) < 4:
            return file_error(i, 'less than four entries'), None, None
        id = 0
        lat = 0
        lng = 0
        ts = 0
        try:
            id = int(entry[0])
        except Exception:
            return file_error(i, 'entry 0 is not an id'), None, None
        if id < 0:
            return file_error(i, 'entry 0 is not an id'), None, None
        try:
            lat = float(entry[1])
        except Exception:
            return file_error(i, 'entry 0 is not a latitude'), None, None
        if lat < -90 or lat > 90:
            return file_error(i, 'entry 0 is not a latitude'), None, None
        try:
            lng = float(entry[2])
        except Exception:
            return file_error(i, 'entry 0 is not a latitude'), None, None
        if lng < -180 or lng > 180:
            return file_error(i, 'entry 0 is not a latitude'), None, None
        try:
            ts = float(entry[3])
        except Exception:
            return file_error(i, 'entry 3 is not a timestamp'), None, None
        if ts < 0:
            return file_error(i, 'entry 3 is not a timestamp'), None, None

        # if log contains rssi values
        if with_rssi:
            for j in range(5, len(entry), 2):
                if id not in edges:
                    edges[id] = {}

                if ts not in edges[id]:
                    edges[id][ts] = {}

                edges[id][ts][int(entry[j - 1])] = entry[j]

        parsed.append((id, lat, lng, ts))

    return None, parsed, edges


def file_error(line, message):
    return {'error': 'Line ' + str(line) + ' - ' + message}


model_folder = "backend/models/"


def list_models():
    res = []

    postfix = ".xml"
    for file in glob.glob(model_folder + "/*" + postfix):
        model = str(file)
        res.append(model[len(model_folder): - len(postfix)])
    return {"models": res}


def get_id(data, first):
    last = first
    while data[last] != ']':
        last += 1
    return (data[first: last], last)


def run_static_grid(model, num_nodes, init_time, duration):
    if num_nodes <= 0:
        return {'error': 'Expected at least one node'}
    if num_nodes >= 10000:
        return {'error': 'Expected less than 10000 nodes'}

    if init_time < 0:
        return {'error': 'Expected at least some init time'}
    if init_time >= duration:
        return {'error': 'Expected duration to be larger than init_time'}
    path = model_folder + "/" + model + ".xml"
    if not os.path.isfile(path):
        return {'error': 'Could not find ' + str(path)}
    p = Popen(['verifyta', "-W", "-s", path], stdout=PIPE, stdin=PIPE, stderr=PIPE, universal_newlines=True)
    lines = str(duration) + " " + str(num_nodes) + " "

    for n in range(num_nodes):
        lines += str(0) + " "
        lines += str(init_time) + " "
        lines += str(duration) + " "
    (stdout, stderr) = p.communicate(input=lines)
    data = parseEngineStdout(stdout)
    minlat = 57.013219
    minlng = 9.991016
    maxlat = 57.017997
    maxlng = 10.001937
    nodes = {}
    square = int(math.sqrt(num_nodes))
    dlat = (maxlat - minlat) / square
    dlon = (maxlng - minlng) / square
    maxlat = minlat
    maxlng = minlng
    for i in range(num_nodes):
        lat = minlat + int(i / square) * dlat
        lng = minlng + int(i % square) * dlon
        maxlat = max(maxlat, lat)
        maxlng = max(maxlng, lng)

    fields = data[0].variables()
    no = 0
    edges = {}
    for field in fields:
        raw = data[0].raw(no)
        if field[7] == 'N':  # OUTPUT_NODES[
            (id, last) = get_id(field, 12)
            lat = minlat + int(int(id) / square) * dlat
            lng = minlng + int(int(id) % square) * dlon
            maxlat = max(maxlat, lat)
            maxlng = max(maxlng, lng)
            field = field[last + 2:]
            if id not in nodes:
                nodes[id] = []
            num = 0
            lastval = 0
            for (ts, val) in raw:
                while num < len(nodes[id]) and nodes[id][num]['timestamp'] < ts:
                    if num is not 0:
                        nodes[id][num][field] = lastval
                    num += 1
                lastval = val
                if num == len(nodes[id]) and num == 0:
                    nodes[id].append({'lat': lat, 'lng': lng, 'timestamp': ts, field: val})
                elif num < len(nodes[id]) and nodes[id][num]['timestamp'] == ts:
                    nodes[id][num][field] = val
                else:
                    nodes[id].insert(num, copy.deepcopy(nodes[id][num - 1]))
                    nodes[id][num][field] = val
                    nodes[id][num]['timestamp'] = ts
        elif field[7] == 'E':  # OUTPUT_EDGE[
            (id, last) = get_id(field, 12)
            (oid, last) = get_id(field, last + 2)
            if id == oid:
                no += 1
                continue
            field = field[last + 2:]
            if id not in edges:
                edges[id] = {}
            if oid not in edges[id]:
                edges[id][oid] = []
            num = 0
            lastval = 0
            for (ts, val) in raw:
                while num < len(edges[id][oid]) and edges[id][oid][num]['timestamp'] < ts:
                    if num is not 0:
                        edges[id][oid][num][field] = lastval
                    num += 1
                lastval = val
                if num == len(edges[id][oid]) and num == 0:
                    edges[id][oid].append({'timestamp': ts, field: val, 'dest': int(oid)})
                elif num < len(edges[id][oid]) and edges[id][oid][num]['timestamp'] == ts:
                    edges[id][oid][num][field] = val
                else:
                    edges[id][oid].insert(num, copy.deepcopy(edges[id][oid][num - 1]))
                    edges[id][oid][num][field] = val
                    edges[id][oid][num]['timestamp'] = ts
        no += 1
    for n in nodes:
        if nodes[n][-1]['timestamp'] != duration:
            nodes[n].append(copy.deepcopy(nodes[n][-1]))
            nodes[n][-1]['timestamp'] = duration
    for n in edges:
        for n2 in edges[n]:
            if edges[n][n2][-1]['timestamp'] != duration:
                edges[n][n2].append(copy.deepcopy(edges[n][n2][-1]))
                edges[n][n2][-1]['timestamp'] = duration
    return {'nodes': nodes, 'edges': edges, 'min_lat': minlat, 'max_lat': maxlat, 'min_lng': minlng, 'max_lng': maxlng,
            'first_time': 0, 'last_time': duration}


def run_gps(fdur, tdur, data):
    if tdur is not -1 and tdur <= fdur:
        return {'error': 'From duration should be smaller than to duration (' + str(fdur) + ", " + str(tdur) + ")"}
    return {'error', 'Not yet implemented'}


def run_log(fdur, tdur, data, log_edges):
    if tdur is not -1 and tdur <= fdur:
        return {'error': 'From duration should be smaller than to duration (' + str(fdur) + ", " + str(tdur) + ")"}

    nodes = {}
    edges = {}
    minlat = math.inf
    maxlat = -math.inf
    minlng = math.inf
    maxlng = -math.inf
    mints = math.inf
    maxts = -math.inf

    for row in data:
        nd = {'lat': row[1], 'lng': row[2], 'timestamp': row[3]}

        if row[3] < fdur or (tdur is not -1 and row[3] > tdur):
            continue

        if row[0] not in nodes:
            nodes[row[0]] = [nd]
        else:
            nodes[row[0]].append(nd)

        minlat = min(minlat, row[1])
        maxlat = max(maxlat, row[1])

        minlng = min(minlng, row[2])
        maxlng = max(maxlng, row[2])

        mints = min(mints, row[3])
        maxts = max(maxts, row[3])

    if len(nodes) == 0:
        return {'error': 'No nodes within the duration (or file is empty or could not be parsed)'}

    for key in nodes:
        nodes[key].sort(key=lambda el: el['timestamp'])

    if log_edges is None:
        for id in nodes:
            for step in nodes[id]:
                for other in nodes:
                    if (id == other
                            or nodes[other][0]['timestamp'] > step['timestamp']
                            or nodes[other][-1]['timestamp'] < step['timestamp']):
                        continue

                    nd = {'timestamp': step['timestamp'], 'dest': other}
                    if id not in edges:
                        edges[id] = {}

                    if other not in edges[id]:
                        edges[id][other] = []

                    edges[id][other].append(nd)

    else:
        if len(log_edges) == 0:
            return {'error': "Log does not contain rssi measurements"}

        for nid in nodes.keys():
            for node in nodes[nid]:
                for nid2 in nodes:
                    if nid == nid2:
                        continue

                    if (node['timestamp'] in log_edges[nid]
                            and node['timestamp'] in log_edges[nid2]
                            and nid2 in log_edges[nid][node['timestamp']]
                            and nid in log_edges[nid2][node['timestamp']]):

                        nd = {'timestamp': node['timestamp'], 'dest': nid2,
                              'rssi': int(log_edges[nid][node['timestamp']][nid2])}
                        nd2 = {'timestamp': node['timestamp'], 'dest': nid,
                               'rssi': int(log_edges[nid2][node['timestamp']][nid])}

                        if nid not in edges:
                            edges[nid] = {}

                        if nid2 not in edges:
                            edges[nid2] = {}

                        if nid not in edges[nid2]:
                            edges[nid2][nid] = []

                        if nid2 not in edges[nid]:
                            edges[nid][nid2] = []

                        edges[nid][nid2].append(nd)
                        edges[nid2][nid].append(nd2)

    for key in edges:
        for k2 in edges[key]:
            edges[key][k2].sort(key=lambda el: el['timestamp'])

    return {'nodes': nodes, 'edges': edges, 'min_lat': minlat, 'max_lat': maxlat, 'min_lng': minlng,
            'max_lng': maxlng,
            'first_time': mints, 'last_time': maxts}
