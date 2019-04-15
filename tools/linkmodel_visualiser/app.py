from typing import Tuple, List, Dict

import os
import json
import math
import numpy as np
import plotly.plotly as py
import plotly.offline
import plotly.graph_objs as go
import plotly.figure_factory as ff
from models import LinkPair, Link


def parse(file: str) -> Dict[float, List[LinkPair]]:
    data = {}
    link_pairs: Dict[float, List[LinkPair]] = {}

    with open(file, 'r') as f:
        for line in f:
            split = line.split(',')
            split[len(split) - 1] = split[len(split) - 1][:-1]
            if len(split) <= 4:
                continue
            # id; lat; lon; timestamp; n0_id; n0_rssi...
            if f'{split[0]}' not in data.keys():
                data[f'{split[0]}'] = {}

            d = {
                'lat': split[1],
                'lon': split[2],
                'nodes': {}
            }

            rssi_fields = split[4:]
            for node in range(0, len(rssi_fields), 2):
                d['nodes'][f'{rssi_fields[node]}'] = rssi_fields[node + 1]

            data[f'{split[0]}'][f'{split[3]}'] = d

    for root_node_id, e_root_node in data.items():
        for timestamp, e_time in e_root_node.items():
            links: List[Link] = []
            node_count = len(e_time['nodes'])
            if not node_count > 0:
                continue

            root_lat = e_time['lat']
            root_lon = e_time['lon']

            node_tuples = [[key, val] for key, val in e_time['nodes'].items()]
            for i, pair in enumerate(node_tuples):
                if timestamp not in data[pair[0]].keys():
                    continue

                lat = data[pair[0]][timestamp]['lat']
                lon = data[pair[0]][timestamp]['lon']
                links.append(Link(root_lat, root_lon, lat, lon, pair[1]))

                for pair2 in node_tuples[i + 1:]:
                    if pair2[0] not in data[pair2[0]].keys():
                        continue

                    links.append(Link(lat, lon, data[pair2[0]][timestamp]['lat'], data[pair2[0]][timestamp]['lon'],
                                      data[pair2[0]][timestamp]['nodes'][pair[0]]))

            link_pairs[float(timestamp)] = []
            for i, l1 in enumerate(links):
                for l2 in links[i + 1:]:
                    link_pairs[float(timestamp)].append(LinkPair(l1, l2))

    return link_pairs


def gen_x_axis() -> List[int]:
    x = []
    i = 0
    while i <= 180:
        x.append(i)
        i += round(math.sqrt(20))
        # i += 1 if i < 16 else round(math.sqrt(20))

    return x


def x_axis_to_string() -> List[str]:
    return [f'{x}]' for x in gen_x_axis()]


def compute_std_dev_and_mean(data: List[LinkPair]) -> Tuple[float, float]:
    """
    Compute the standard deviation and mean for sample data

    :param data: The LinkPairs
    :return: [standard deviant, mean]
    """
    data = [i.rssi for i in data]
    mean = sum(data) / len(data)

    x = 0
    for i in data:
        x += pow(mean - i, 2)

    return math.sqrt(x / (len(data) - 1)), mean


if __name__ == '__main__':
    log_path = '../../src/logs/parsed'
    logs = os.listdir(log_path)
    link_pairs = {}

    for log in logs:
        # f_name, f_ext = os.path.splitext(log)
        link_pairs[log] = parse(f'{log_path}/{log}')

    link_pairs_org = link_pairs['original']

    for log in [i for i in logs if not i == 'original']:
        time = 497240000.0

        rssi_traces = []
        org_rssi_data = []
        rssi_data = []
        rssi_group = []

        normal_traces = []
        normal_group_label = []

        sample_size_traces = []

        angle_buckets = gen_x_axis()
        for i, bucket in enumerate(angle_buckets):
            if i == 0:
                org_bucket_data = [j for j in link_pairs_org[time] if j.angle <= bucket]
                bucket_data = [j for j in link_pairs[log][time] if j.angle <= bucket]
            else:
                org_bucket_data = [j for j in link_pairs_org[time] if angle_buckets[i - 1] < j.angle <= bucket]
                bucket_data = [j for j in link_pairs[log][time] if angle_buckets[i - 1] < j.angle <= bucket]

            if len(org_bucket_data) < 1 or len(bucket_data) < 1:
                continue

            # compute the sample size for each bucket
            sample_size_traces.append(len(org_bucket_data))

            # compute the rssi traces
            org_avg = 0
            for j in org_bucket_data:
                org_avg += j.rssi
            org_rssi_data.append(org_avg / len(org_bucket_data))

            avg = 0
            for j in bucket_data:
                avg += j.rssi
            rssi_data.append(avg / len(bucket_data))
            rssi_group.append(bucket)

            # compute the normal distribution for each bucket
            if len(org_bucket_data) > 1:
                std_dev, mean = compute_std_dev_and_mean(org_bucket_data)
                normal_dist = np.random.normal(mean, std_dev, 10000)
                normal_traces.append(normal_dist)
                normal_group_label.append(f'{bucket}]')

        # add the rssi traces
        rssi_traces.append(go.Scatter(
            x=rssi_group,
            y=org_rssi_data,
            name='Original',
            mode='lines+markers'
        ))
        rssi_traces.append(go.Scatter(
            x=rssi_group,
            y=rssi_data,
            name=log,
            mode='lines+markers'
        ))

        # add the sample size trace
        rssi_traces.append(go.Bar(
            x=rssi_group,
            y=sample_size_traces,
            width=0.3,
            yaxis='y2',
            name='Sample size'
        ))
        layout = go.Layout(
            xaxis=go.layout.XAxis(
                tickmode='array',
                tickvals=gen_x_axis(),
                ticktext=x_axis_to_string(),
                showline=True,
                title='Angle buckets'
            ),
            yaxis=go.layout.YAxis(
                showline=True,
                showgrid=False,
                dtick=5,
                title='RSSI'
            ),
            yaxis2=dict(
                side='right',
                title='Sample size',
                overlaying='y',
                showline=True,
                tickmode='auto',
                showgrid=False
            ),
            showlegend=True,
            title=f'Heuristic comparison'
        )
        fig = go.Figure(data=rssi_traces, layout=layout)
        rssi_div = plotly.offline.plot(fig, include_plotlyjs=True, output_type='div')

        # add the normal distribution graph
        normal_fig = ff.create_distplot(normal_traces, normal_group_label, curve_type='normal', show_rug=False,
                                        show_hist=False)
        normal_fig['layout'].update(title='Normal Distributions', xaxis=dict(title='RSSI'),
                                    yaxis=dict(title='Probability density'))
        normal_div = plotly.offline.plot(normal_fig, include_plotlyjs=False, output_type='div')

        with open(f'plots/Original-{log}.html', 'w') as f:
            f.writelines(rssi_div)
            f.writelines(normal_div)

    print("done")
