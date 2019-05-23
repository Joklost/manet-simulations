from typing import Tuple, List, Dict, NoReturn, Union

import os
import statistics
import pprint
import math
import numpy as np
import plotly.plotly as py
import plotly.offline
import plotly.graph_objs as go
import plotly.figure_factory as ff
import sys
import pandas as pd

from models import LinkPair, Link
from angle_rssi import parse_angle_bucket_format, make_dirt_plot
import helpers


def create_linkpairs(file: str, as_pair: bool = True, lower_bound=-1000, upper_bound=1000,
                     remove_distance: bool = False) -> Union[Dict[float, List[LinkPair]], List[Link]]:
    data = {}
    link_pairs: Dict[float, List[LinkPair]] = {}
    all_links: List[Link] = []

    with open(file, 'r') as f:
        for line in f:
            split = line.split(',')
            split[len(split) - 1] = split[len(split) - 1][:-1]
            if len(split) <= 4:
                continue
            # id; lat; lon; timestamp; n0_id; n0_rssi...
            if str(split[0]) not in data.keys():
                data[f'{split[0]}'] = {}

            d = {
                'lat': split[1],
                'lon': split[2],
                'nodes': {}
            }

            rssi_fields = split[4:]
            for node in range(0, len(rssi_fields), 2):
                d['nodes'][str(rssi_fields[node])] = rssi_fields[node + 1]

            data[str(split[0])][str(split[3])] = d

    for id_root_node, e_root_node in data.items():
        for timestamp, e_time in e_root_node.items():
            links: List[Link] = []
            node_count = len(e_time['nodes'])
            if not node_count > 0:
                continue

            root_lat = e_time['lat']
            root_lon = e_time['lon']

            node_tuples = [[key, val] for key, val in e_time['nodes'].items()]
            for i, pair in enumerate(node_tuples):
                if pair[0] not in data.keys() or timestamp not in data[pair[0]].keys():
                    continue

                lat = data[pair[0]][timestamp]['lat']
                lon = data[pair[0]][timestamp]['lon']

                if not float(pair[1]) == -128.0 and lower_bound <= float(pair[1]) <= upper_bound:
                    links.append(Link(root_lat, root_lon, lat, lon, pair[1], timestamp, id_root_node, pair[0],
                                      remove_distance=remove_distance))

                # for pair2 in node_tuples[i + 1:]:
                #     if pair2[0] not in data.keys() or pair2[0] not in data[pair2[0]].keys():
                #         continue
                #
                #     links.append(Link(lat, lon, data[pair2[0]][timestamp]['lat'], data[pair2[0]][timestamp]['lon'],
                #                       data[pair2[0]][timestamp]['nodes'][pair[0]]))

            if as_pair:
                link_pairs[float(timestamp)] = []
                for i, l1 in enumerate(links):
                    for l2 in links[i + 1:]:
                        if l1.has_common_node(l2):
                            link_pairs[float(timestamp)].append(LinkPair(l1, l2))
            else:
                all_links += links

    return link_pairs if as_pair else all_links


def _gen_angle_buckets(interval: int = 5) -> List[int]:
    return [i for i in range(interval, 181, interval)]


def _str_angle_bucket(interval: int = 5) -> List[str]:
    return [f'0-{interval}'] + [f'{i - interval + 1}-{i}' for i in range(interval * 2, 181, interval)]


def _str_distance_bucket(upper_bound: int, lower_bound: int = 0, interval: int = 20) -> List[str]:
    return [f'0-{interval}'] + [f'{i - interval + 1}-{i}' for i in range(lower_bound * 2, upper_bound, interval)]


def parse_to_angle_bucket_individual_rssi_distance(data: Dict[float, List[LinkPair]], as_average: bool = False) \
        -> Dict[int, List[Tuple[float, float]]]:
    angle_buckets = _gen_angle_buckets()

    sorted_data = {key: [] for key in angle_buckets}
    for time in data.keys():
        for i, angle in enumerate(angle_buckets):
            bucket_data = []
            for j in data[time]:
                if j.angle <= angle if i is 0 else angle_buckets[i - 1] < j.angle <= angle:
                    if as_average:
                        bucket_data.append(j)
                    else:
                        bucket_data.append([j.link1.rssi, j.link1.distance])
                        bucket_data.append([j.link2.rssi, j.link2.distance])

            sorted_data[angle] = sorted_data[angle] + bucket_data

    return sorted_data


def parse_logfile_name(log: str) -> str:
    return os.path.split(log)[1].split('.')[0]


def parse_linkpairs_to_angle_bucket_sorted(data, interval=None) -> Tuple[int, Dict[int, List[float]]]:
    angle_buckets = _gen_angle_buckets() if interval is None else _gen_angle_buckets(interval=interval)
    fixed = {key: [] for key in angle_buckets}
    count = 0

    for time in data.keys():
        for i, angle in enumerate(angle_buckets):
            bucket_data = []
            for lp in data[time]:
                if lp.angle <= angle if i == 0 else angle_buckets[i - 1] < lp.angle <= angle:
                    # if lp.link2.distance > 9 and lp.link1.distance > 9: # min distance
                    bucket_data.append(lp.rssi_diff)

            fixed[angle] += bucket_data
            count += len(bucket_data)

    return count, fixed


def build_average_rssi_for_distance_plot(log: str):
    measured_log = create_linkpairs(log, as_pair=False)
    bar_y = []
    scatter_y = []
    x = []

    count = 0
    prev = 0
    max_dist = max([val.distance for val in measured_log])
    for dist in range(0, int(max_dist), 20):
        bucket = [link.rssi for link in measured_log if prev < link.distance <= dist]

        if len(bucket) < 1:
            prev = dist
            continue

        count += len(bucket)
        avg_rssi = sum(bucket) / len(bucket)

        bar_y.append(avg_rssi)
        x.append(f'{prev + 1}-{dist}')
        prev = dist

    # print(list(zip(range(len(bar_y)), bar_y)))
    # print(list(zip(range(len(bar_y)), [26 - Link.l_d(dist) for dist in range(1, int(max_dist), 20)])))

    data = [go.Bar(
        x=x,
        y=bar_y,
        name='Average RSSI',
        width=0.2
    ), go.Scatter(
        x=x,
        y=[26 - Link.l_d(dist) for dist in range(1, int(max_dist), 20)],
        name='Distance function'
    )]

    log_name = parse_logfile_name(log)
    layout = go.Layout(
        title=f'{log_name} data - The average RSSI in each distance bucket raw - Sample size: {count}',
        xaxis=dict(
            title='Distance bucket'
        ),
        yaxis=dict(
            title='Average RSSI'
        )
    )
    fig = go.Figure(data=data, layout=layout)
    plotly.offline.plot(fig, filename=f'plots/The_average_RSSI_in_each_distance_bucket_sep{log_name}.html')


def build_angle_bucket_stochastic_minus_distance_fading_increase_plot(log: str):
    measured_log = create_linkpairs(log, remove_distance=True)
    count, fixed = parse_linkpairs_to_angle_bucket_sorted(measured_log)

    x = []
    y = []
    for i, angle in enumerate(_gen_angle_buckets()):
        if len(fixed[angle]) < 1:
            continue

        x.append(_str_angle_bucket()[i])
        y.append(sum(fixed[angle]) / len(fixed[angle]))

    data = [go.Bar(
        x=x,
        y=y
    )]

    log_name = parse_logfile_name(log)
    layout = go.Layout(
        title=f'{log_name} data - average RSSI minus distance fading for angle buckets - sample size: {count}',
        xaxis=dict(
            title='Angle bucket'
        ),
        yaxis=dict(
            title='Average RSSI minus distance'
        )
    )
    fig = go.Figure(data=data, layout=layout)
    plotly.offline.plot(fig, filename=f'plots/average_RSSI_minus_distance_fading_for_angle_buckets_{log_name}.html')


def build_normal_dist_for_angle_bucket(log: str):
    measured_log = create_linkpairs(log, remove_distance=True)
    count, fixed = parse_linkpairs_to_angle_bucket_sorted(measured_log)
    data = []
    group_labels = []

    for i, angle in enumerate(_gen_angle_buckets()):
        if len(fixed[angle]) < 1:
            continue

        std_dev, mean = helpers.compute_std_dev_and_mean(fixed[angle])
        data.append(np.random.normal(mean, std_dev, 10000))

        group_labels.append(_str_angle_bucket()[i])

    log_name = parse_logfile_name(log)
    fig = ff.create_distplot(data, group_labels, curve_type='normal', show_rug=False,
                             show_hist=False, show_curve=True)
    fig['layout'].update(
        title=f'{log_name} data - Normal distributions for raw RSSI measurements for angle buckets - Sample size: {count}',
        xaxis=dict(title='RSSI'),
        yaxis=dict(title='Probability density'))
    # data = [go.Histogram(
    #
    # )]

    plotly.offline.plot(fig,
                        filename=f'plots/normal_distributions_for_raw_RSSI_measurements_for_angle_buckets_{log_name}.html')


def build_normal_dist_for_distance_bucket(log: str):
    measured_log = create_linkpairs(log, as_pair=False, remove_distance=True)

    data = []
    group_labels = []
    count = 0
    prev = 0
    for distance in range(0, int(max([val.distance for val in measured_log])), 50):
        bucket = [link.rssi for link in measured_log if prev < link.distance <= distance]
        if len(bucket) < 1:
            continue
            prev = distance

        std_dev, mean = helpers.compute_std_dev_and_mean(bucket)

        # if prev in [100, 300, 500, 650]:
        #     print(f'prev: {prev} - dist. {distance} - std_dev: {std_dev} - mean: {mean}')

        data.append(np.random.normal(mean, std_dev, 10000))
        group_labels.append(f'{prev + 1}-{distance}')
        prev = distance
        count += len(bucket)

    log_name = parse_logfile_name(log)
    fig = ff.create_distplot(data, group_labels, curve_type='normal', show_rug=False,
                             show_hist=False, show_curve=True)
    fig['layout'].update(
        title=f'{log_name} data - Normal distributions for raw RSSI measurements for distance buckets - Sample size: {count}',
        xaxis=dict(title='RSSI'),
        yaxis=dict(title='Probability density'))

    plotly.offline.plot(fig,
                        filename=f'plots/normal_distributions_for_raw_RSSI_measurements_for_distance_buckets_{log_name}.html')


def build_histogram_for_stochastic_in_angle_buckets(log: str):
    measured_log = create_linkpairs(log, remove_distance=True)
    count, fixed = parse_linkpairs_to_angle_bucket_sorted(measured_log, interval=3)

    hist_data = []
    table_angle = []
    table_avg_rssi = []
    table_median = []
    for i, angle in enumerate(_gen_angle_buckets(interval=3)):
        if angle not in fixed.keys() or len(fixed[angle]) < 1:
            continue

        data = fixed[angle]
        hist_data.append(go.Histogram(
            # x=[val / sum(fixed[angle]) for val in fixed[angle]], # normalized value
            x=data,
            name=_str_angle_bucket(interval=3)[i],
            histnorm='percent'  # count presented as percentage
        ))
        table_angle.append(_str_angle_bucket(interval=3)[i])
        table_avg_rssi.append(sum(data) / len(data))
        table_median.append(statistics.median(data))

    log_name = parse_logfile_name(log)
    hist_layout = go.Layout(
        xaxis=dict(
            title='Difference in RSSI minus distance fading between a link pair'
        ),
        yaxis=dict(
            title='Percentage'
        ),
        title=f'{log_name} - Raw - Sample size: {count}'
    )
    hist_fig = go.Figure(data=hist_data, layout=hist_layout)
    hist_div = plotly.offline.plot(hist_fig, include_plotlyjs=True, output_type='div')

    table_trace = [go.Table(
        header=dict(
            values=['Angle', 'Average RSSI', 'Median'],
            line=dict(color='#7D7F80'),
            fill=dict(color='#a1c3d1'),
            align=['left'] * 5
        ),
        cells=dict(
            values=[table_angle, table_avg_rssi, table_median],
            align=['left'] * 5
        )
    )]
    table_layout = dict(width=800)
    table_fig = go.Figure(data=table_trace, layout=table_layout)
    table_div = plotly.offline.plot(table_fig, include_plotlyjs=False, output_type='div')

    with open(f'plots/histogram_raw_{log_name}.html', 'w') as f:
        f.write(hist_div)
        f.write(table_div)


def build_scatter_for_least_square_losmodel(log: str):
    measured_log = create_linkpairs(log, as_pair=False, remove_distance=True)
    plot_data = create_linkpairs(log, as_pair=False)

    max_dist = int(max([val.distance for val in measured_log]))
    prev = 0
    x = []
    y = []
    plot_y = []

    for dist in range(0, max_dist, 20):
        bucket = [link.rssi for link in measured_log if prev < link.distance <= dist]
        plot_bucket = [link.rssi for link in plot_data if prev < link.distance <= dist]

        if len(bucket) < 1:
            prev = dist
            continue

        y.append(sum(bucket) / len(bucket))
        plot_y.append(sum(plot_bucket) / len(plot_bucket))
        x.append(dist)

    losmodel = helpers.compute_least_squared_regression(x, y)
    print([losmodel(d) for d in x])
    # helpers.compute_total_least_squared_regression(x, y)
    #
    # print(log)
    # print(m)
    # print(c)
    # print(y)
    # print(x)
    #
    data = [go.Scatter(
        x=x,
        y=plot_y,
        name='Average RSSI for distance bucket',
        error_y=dict(
            type='data',
            array=[losmodel(i) for i in x],
            visible=True
        )
    ), go.Scatter(
        x=[i for i in range(max_dist)],
        y=[26 - Link.l_d(i) for i in range(max_dist)],
        name='Distance function'
    # )]
    ), go.Scatter(
        x=[i for i in range(max_dist)],
        y=[losmodel(i) for i in range(max_dist)],
        name='LoSModel'
    )]

    layout = go.Layout(
        title=parse_logfile_name(log)
    )
    fig = go.Figure(data=data, layout=layout)
    plotly.offline.plot(fig, filename='plots/scatter_least_squared_for_average_RSSI.html')


if __name__ == '__main__':
    # np.seterr(divide='ignore', invalid='ignore')
    print('Loading data from logs')
    for arg in sys.argv[2:]:
        # build plot presenting the average stochastic for angle buckets
        # build_angle_bucket_stochastic_minus_distance_fading_increase_plot(arg)

        # build normal distribution plot for angle buckets with distance fading
        # build_normal_dist_for_angle_bucket(arg)

        # build normal distribution plot for distance buckets with distance fading
        # build_normal_dist_for_distance_bucket(arg)

        # build histogram for frequency of stochastic value only in angle buckets
        # build_histogram_for_stochastic_in_angle_buckets(arg)

        # build bar plot for average rssi in a distance bucket
        # build_average_rssi_for_distance_plot(arg)

        # build scatter plot for least square regression for LoSModel
        build_scatter_for_least_square_losmodel(arg)
