from typing import Tuple, List, Dict, NoReturn

import os
import pprint
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

    for _, e_root_node in data.items():
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

    return x


def x_axis_to_string() -> List[str]:
    return [f'{x}]' for x in gen_x_axis()]


def compute_std_dev_and_mean(data: List[float]) -> Tuple[float, float]:
    """
    Compute the standard deviation and mean for sample data

    :param data: The LinkPairs
    :return: [standard deviantion, mean]
    """
    mean = sum(data) / len(data)

    x = 0
    for i in data:
        x += pow(mean - i, 2)

    return math.sqrt(x / (len(data) - 1)), mean


def format_trace_data(data: Dict[float, List[LinkPair]]) -> Dict[int, List[float]]:
    angle_buckets = gen_x_axis()

    # parse data to dict {angle: sample within angle range}
    sorted_data = {key: [] for key in angle_buckets}
    for time in data.keys():
        for i, angle in enumerate(angle_buckets):
            bucket_data = [j.rssi for j in data[time] if j.angle <= angle] if i == 0 else \
                [j.rssi for j in data[time] if angle_buckets[i - 1] < j.angle <= angle]
            sorted_data[angle] = sorted_data[angle] + bucket_data

    return sorted_data


def build_normal_plot(data: Dict[int, List[float]]) -> plotly.offline.plot:
    traces = []
    group_label = []

    for angle, bucket in data.items():
        if len(bucket) > 1:
            std_dev, mean = compute_std_dev_and_mean(bucket)
            normal_dist = np.random.normal(mean, std_dev, 10000)
            traces.append(normal_dist)
            group_label.append(f'{angle}]')

    fig = ff.create_distplot(traces, group_label, curve_type='normal', show_rug=False,
                             show_hist=False)
    fig['layout'].update(title='Normal Distributions', xaxis=dict(title='RSSI'),
                         yaxis=dict(title='Probability density'))
    return plotly.offline.plot(fig, include_plotlyjs=False, output_type='div')


def build_sample_size_trace(data: Dict[int, List[float]]) -> go.Bar:
    """
    Build an sample size trace

    :param data: The data
    :return: plotly Bar trace
    """
    group = []
    samples = []
    for angle, bucket in data.items():
        samples.append(len(bucket))
        group.append(angle)

    return go.Bar(
        x=group,
        y=samples,
        width=0.3,
        yaxis='y2',
        name='Sample size'
    )


def build_rssi_trace(data: Dict[int, List[float]], name: str) -> go.Scatter:
    """
    Build an RSSI trace

    :param data: The data
    :param name: Name of the trace to appear in legend
    :return: plotly Scatter trace
    """
    group = []
    rssi = []

    for angle in gen_x_axis():
        bucket = data[angle]
        if len(bucket) < 1:
            continue

        avg = 0
        for j in bucket:
            avg += j
        rssi.append(avg / len(bucket))
        group.append(angle)

    return go.Scatter(
        x=group,
        y=rssi,
        name=name,
        mode='lines+markers'
    )


def compute_score(data: Dict[int, List[float]], org_data: Dict[int, List[float]]) -> float:
    """
    Compute the score based on the difference of each RSSI value

    :return: Score
    """
    score = 0.0
    for angle, bucket in data.items():
        if len(bucket) < 1 or len(org_data[angle]) < 1:
            continue

        avg = 0.0
        for val in bucket:
            avg += val
        avg = avg / len(bucket)

        org_avg = 0.0
        for val in org_data[angle]:
            org_avg += val
        org_avg = org_avg / len(org_data[angle])

        score += math.pow(abs(org_avg - avg), 2)

    return score


def build_traces(data: Dict[float, List[LinkPair]], org_data: Dict[int, List[float]], log: str) -> NoReturn:
    print('Parsing data')
    sorted_data = format_trace_data(data)

    # TODO: write to debug log
    # with open('debug_log_computed', 'w') as f:
    #     for val in sorted_data[4]:
    #         f.write(f'{val}\n')

    # break if all buckets are empty. This happens when the network is initialized
    is_empty = True
    for angle, bucket in sorted_data.items():
        if len(bucket) > 1:
            is_empty = False

    if is_empty:
        return

    # build sample size trace
    print('Building sample size trace')
    sample_size_trace = build_sample_size_trace(sorted_data)

    # build heuristic trace
    print('Building heuristic trace')
    heuristic_trace = build_rssi_trace(sorted_data, log)

    # build org trace
    print('Building log measurement trace')
    org_trace = build_rssi_trace(org_data, 'original')

    # build normal distribution plot
    print('Building normal distribution trace')
    normal_plot = build_normal_plot(sorted_data)

    # compute the score
    print('Computing the score')
    score = compute_score(sorted_data, org_data)

    layout = go.Layout(
        xaxis=go.layout.XAxis(
            tickmode='array',
            tickvals=gen_x_axis(),
            ticktext=x_axis_to_string(),
            showline=False,
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
        title=f'Heuristic comparison - Score: {round(score, 2)}'
    )

    fig = go.Figure(data=[org_trace, heuristic_trace, sample_size_trace], layout=layout)
    rssi_div = plotly.offline.plot(fig, include_plotlyjs=True, output_type='div')

    with open(f'plots/original-{log}.html', 'w') as f:
        f.writelines(rssi_div)
        f.writelines(normal_plot)


if __name__ == '__main__':
    log_path = '../../src/logs/parsed'
    logs = os.listdir(log_path)
    link_pairs = {}

    print('Loading data from logs')
    for log in logs:
        if log.endswith('.csv'):
            f_name, _ = os.path.splitext(log)
        else:
            f_name = log

        link_pairs[f_name] = parse(f'{log_path}/{f_name}')

    for log in [i for i in logs if not i == 'original']:
        if log.endswith('.csv'):
            f_name, _ = os.path.splitext(log)
        else:
            f_name = log

        build_traces(link_pairs[f_name], format_trace_data(link_pairs['original']), f_name)

    print("done")