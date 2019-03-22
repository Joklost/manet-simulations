




# validate data and proxy to real functions
def execute(data):
    if 'type' not in data:
        return {'error':'No type recieved'}
    if data['type'] == 'static':
        if 'topology' in data:
            if data['topology'] == 'grid':
                if 'number_of_nodes' in data and 'node_init_time' in data and 'duration' in data:
                    run_static_grid(data['number_of_nodes'], data['node_init_time'], data['duration'])
                else:
                    return {'error': 'Missing arguments for simulation'}
            else:
                return {'error': 'Uknown topology'}
        else:
            return {'error': "No topology recieved"}
    return {'error': "Unknown type or topology"}


def run_static_grid(num_nodes, init_time, duration):
    if num_nodes is None or num_nodes.isdigit() or num_nodes <= 0:
        return {'error': 'Expected at least one node'}
    if num_nodes >= 10000:
        return {'error': 'Expected less than 10000 nodes'}

    if init_time is None or init_time.isdigit() or init_time < 0:
        return {'error': 'Expected at least some init time'}
    if init_time >= duration:
        return {'error': 'Expected duration to be larger than init_time'}