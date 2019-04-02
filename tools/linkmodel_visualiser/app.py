import os
import json
from typing import Tuple

import math
import numpy as np
import plotly.plotly as py
import plotly.offline
import plotly.graph_objs as go


def angle_between(origin: Tuple[float, float], location1: Tuple[float, float], location2: Tuple[float, float]) -> float:
    origin_radian = np.radians(np.array(origin))
    loc1_radian = np.radians(np.array(location1))
    loc2_radian = np.radians(np.array(location2))

    loc1_vec = loc1_radian - origin_radian
    loc2_vec = loc2_radian - origin_radian

    loc1_vec[1] *= math.cos(origin_radian[0])
    loc2_vec[1] *= math.cos(origin_radian[0])

    return np.degrees(math.acos(np.dot(loc1_vec, loc2_vec) / (np.linalg.norm(loc1_vec) * np.linalg.norm(loc2_vec))))


def main():
    log_path = '../../src/logs/parsed'
    logs = os.listdir(log_path)

    for log in logs:
        data = {}
        with open(f'{log_path}/{log}') as f:
            for line in f:
                if len(split) <= 4:
                    continue

                split = line.split(';')
                d = {
                    'lat': split[1],
                    'lon': split[2],
                    'nodes': {}
                }

                rssi_fields = split[4:]
                for node in range(0, len(rssi_fields), 2):
                    d[f'{split[3]}']['nodes'][f'{rssi_fields[node]}'] = rssi_fields[node + 1]

                data[f'{split[0]}'][f'{split[3]}'] = d

        x = []
        y = []
        for root_node_id, e_root_node in data.items():
            for timestamp, e_time in e_root_node.items():
                if not len(e_time['nodes'] > 0):
                    continue

                root_lat = e_time['lat']
                root_lon = e_time['lon']

                for node_id, e_node in e_time['nodes'].items():
                    node_lat = data[node_id][timestamp]['lat']
                    node_lon = data[node_id][timestamp]['lon']



        # if df == 'nodes':
        #     plot_data.append(go.Bar(
        #         x=content['id'],
        #         y=content['count'],
        #         width=0.01,
        #         name=df
        #     ))
        # else:
        plot_data.append(go.Scatter(
            x=[],
            y=[i for i in range(0, 361)],
            name=log
        ))

    plotly.offline.plot(plot_data, filename='model.html')


if __name__ == '__main__':
    main()
