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
    total_bi_directional_links = 0

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
                    bi_directional = False
                    for l2 in links:
                        if l1 == l2:
                            continue

                        if l1.has_common_node(l2):
                            bi_directional = True
                            link_pairs[float(timestamp)].append(LinkPair(l1, l2))

                    if not bi_directional:
                        total_bi_directional_links += 1
            else:
                all_links += links

    print(f'Amount of none bi-directional links: {total_bi_directional_links}')
    return link_pairs if as_pair else all_links


def _gen_angle_buckets(interval: int = 5) -> List[int]:
    return [i for i in range(interval, 181, interval)]


def _str_angle_bucket(interval: int = 5) -> List[str]:
    return [f'0-{interval}'] + [f'{i - interval + 1}-{i}' for i in range(interval * 2, 181, interval)]


def _str_distance_bucket(upper_bound: int, lower_bound: int = 0, interval: int = 20) -> List[str]:
    return [f'0-{interval}'] + [f'{i - interval + 1}-{i}' for i in range(interval, upper_bound + interval, interval)]


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
    bar_y, x, dist_x = [], [], []

    count = 0
    prev = 0
    max_dist = max([val.distance for val in measured_log]) + 20
    for dist in range(0, int(max_dist), 20):
        bucket = [link.rssi for link in measured_log if prev < link.distance <= dist]
        prev = dist

        if len(bucket) < 1:
            continue

        count += len(bucket)
        avg_rssi = sum(bucket) / len(bucket)

        bar_y.append(avg_rssi)
        x.append(f'{prev + 1}-{dist}')
        dist_x.append(dist)

    # print(list(zip(range(len(bar_y)), bar_y)))
    # print(list(zip(range(len(bar_y)), [26 - Link.l_d(dist) for dist in range(1, int(max_dist), 20)])))

    print(x)
    print(dist_x)
    print(list(zip(dist_x, bar_y)))
    print(list(zip(dist_x, [26 - Link.l_d_org(dist) for dist in dist_x])))
    print()

    data = [go.Scatter(
        x=x,
        y=bar_y,
        name='Average RSSI'
    ), go.Scatter(
        x=x,
        y=[26 - Link.l_d_org(dist) for dist in dist_x],
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
    plotly.offline.plot(fig, filename=f'plots/The_average_RSSI_in_each_distance_bucket_{log_name}.html')


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

    data = [go.Scatter(
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
    plotly.offline.plot(fig, filename=f'plots/scatter_average_RSSI_minus_distance_fading_for_angle_buckets_{log_name}.html')


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
    hist_div = plotly.offline.plot(hist_fig, t)

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


def compute_least_square(log: str):
    measured_log = create_linkpairs(log, as_pair=False, remove_distance=True)
    plot_data = create_linkpairs(log, as_pair=False)

    max_dist = int(max([val.distance for val in measured_log]))
    prev = 0
    x = []
    y = []
    plot_y = []

    for dist in range(0, max_dist, 20):
        bucket = [link.rssi for link in measured_log if prev < link.distance <= dist]
        #     plot_bucket = [link.rssi for link in plot_data if prev < link.distance <= dist]
        #
        if len(bucket) < 1:
            prev = dist
            continue
        #
        y.append(sum(bucket) / len(bucket))
        #     plot_y.append(sum(plot_bucket) / len(plot_bucket))
        x.append(dist)
        prev = dist

    # x = [link.rssi for link in measured_log]
    # y = [val for val in range(max_dist)]
    losmodel, m, c = helpers.compute_least_squared_regression(x, y)
    # helpers.compute_total_least_squared_regression(x, y)
    #
    print(log)
    print(f'm={m}')
    print(f'c={c}')


def build_scatter_for_least_square_losmodel_v2(log: str):
    # losmodel_links = helpers.load_losmodel_computed_rssi_data('losmodel_link_data.json')
    # losmodel_links_new = helpers.load_losmodel_computed_rssi_data('losmodel_link_data_new.json')
    # losmodel_links_less_5 = helpers.load_losmodel_computed_rssi_data('losmodel_link_data_less_5.json')
    measurement_raw_phili = create_linkpairs(log, as_pair=False, remove_distance=False)
    measurement_raw_rude = create_linkpairs('../gpslogs/rude_skov_gps_rssi.txt', as_pair=False, remove_distance=False)
    measurement_distance = create_linkpairs(log, as_pair=False, remove_distance=True)

    data_sets = [
        ['phili measurements', measurement_raw_phili],
        ['rude measurements', measurement_raw_rude],
        ['measurements distance', measurement_distance],
    ]

    los_y = []
    los_y_new = []
    los_less_5 = []
    raw_y = []
    x = []
    y = []
    raw_rude = []

    plot_data = []

    max_dist = int(max([val.distance for val in measurement_raw_phili])) + 20
    for name, data in data_sets:
        scatter, y, x = [], [], []
        prev = 0

        for i, dist in enumerate(range(0, max_dist, 20)):
            bucket = [link.rssi for link in data if prev < link.distance <= dist]
            prev = dist

            if len(bucket) < 1:
                continue

            y.append(sum(bucket) / len(bucket))
            x.append(_str_distance_bucket(max_dist)[i])

        plot_data.append(go.Scatter(
            x=x,
            y=y,
            name=name
        ))

    for i, dist in enumerate(range(0, max_dist, 20)):
        bucket = [link.rssi for link in measurement_distance if prev < link.distance <= dist]
        # bucket_rude = [link.rssi for link in measurement_distance_rude if prev < link.distance <= dist]
        raw_bucket = [link.rssi for link in measurement_raw_phili if prev < link.distance <= dist]
        # los_bucket = [link.rssi for link in losmodel_links if prev < link.distance <= dist]
        # los_bucket_new = [link.rssi for link in losmodel_links_new if prev < link.distance <= dist]
        # los_bucket_less_5 = [link.rssi for link in losmodel_links_less_5 if prev < link.distance <= dist]

        if len(bucket) < 1 or len(raw_bucket) < 1:
            # if len(bucket) < 1 or len(raw_bucket) < 1 or len(los_bucket) < 1 or len(los_bucket_less_5) < 1:
            prev = dist
            continue

        prev = dist

        # print(f'min: {min(los_bucket)}, max: {max(los_bucket)}')
        # print(sorted([f'{val}: {los_bucket.count(val)}' for val in los_bucket]))

        y.append(sum(bucket) / len(bucket))
        raw_y.append(sum(raw_bucket) / len(raw_bucket))
        # raw_rude.append(sum(bucket_rude) / len(bucket_rude))
        # los_y.append(sum(los_bucket) / len(los_bucket))
        # los_y_new.append(sum(los_bucket_new) / len(los_bucket_new))

        # los_less_5.append(sum(los_bucket_less_5) / len(los_bucket_less_5))
        x.append(_str_distance_bucket(max_dist)[i])

    plot_data.append(
        go.Scatter(
            x=[dist for dist in range(0, max_dist, 20)],
            y=[26 - Link.l_d(val) for val in range(0, max_dist, 20)],
            name='CVPL function'
        )
    )
    plot_data.append(
        go.Scatter(
            x=[dist for dist in range(0, max_dist, 20)],
            y=[26 - Link.l_d_org(val) for val in range(0, max_dist, 20)],
            name='Original distance function'
        )
    )
    plot_data.append(
        go.Scatter(
            x=[dist for dist in range(0, max_dist, 20)],
            y=[26 - Link.los_fading_v2(val) for val in range(0, max_dist, 20)],
            name='LoS function for distance buckets(interval=20)'
        )
    )

    data = [go.Scatter(
        x=x,
        y=raw_y,
        name='Measurements phili'
    ), go.Scatter(
        x=x,
        y=[26 - Link.l_d(val) for val in range(0, max_dist, 20)],
        name='CVPL function'
    ), go.Scatter(
        x=x,
        y=[26 - Link.l_d_org(val) for val in range(0, max_dist, 20)],
        name='Original distance function'
    ),

        # go.Scatter(
        # x=x,
        # y=los_y,
        # name='LoS computation for measurements'
        # )
        go.Scatter(
            x=x,
            y=[26 - Link.los_fading_v2(val) for val in range(0, max_dist, 20)],
            name='LoS function for distance buckets(interval=20)'
        ),
        # go.Scatter(
        # x=x,
        # y=los_less_5,
        # name='Link with less than 5% building'
        # )
        # , go.Scatter(
        #     x=x,
        #     y=raw_rude,
        #     name='Measurement rudeskov'
        # )
        #     , go.Scatter(
        #     x=x,
        #     y=los_y_new,
        #     name='LoS2 computation for measurements'
        # ), go.Scatter(
        #     x=x,
        #     y=[26 - Link.los_fading_v2(val) for val in range(max_dist)],
        #     name='LoS2 function for all links'
        # )
    ]

    layout = go.Layout(
        xaxis=dict(
            title='Distance buckets'
        ), yaxis=dict(
            title='RSSI'
        )
    )
    f_name = parse_logfile_name(log)
    plotly.offline.plot(go.Figure(data=plot_data, layout=layout),
                        filename=f'plots/scatter_losmodel_comparison_to_measurements_{f_name}.html')


def build_scatter_compare_field_cvpl_plus_bopl(log):
    measurement_raw_phili = create_linkpairs(log, as_pair=False, remove_distance=False)
    print(len(measurement_raw_phili))
    computed = create_linkpairs('phili_computed_gps_rssi.txt', as_pair=False,
                                remove_distance=False)

    max_dist = int(max([val.distance for val in measurement_raw_phili])) + 20
    prev, y, comp_y, x, s_x = 0, [], [], [], []

    for i, dist in enumerate(range(0, max_dist, 20)):
        bucket = [link.rssi for link in measurement_raw_phili if prev < link.distance <= dist]
        computed_bucket = [link.rssi for link in computed if prev < link.distance <= dist]
        prev = dist

        if len(bucket) < 1 or len(computed_bucket) < 1:
            continue

        y.append(sum(bucket) / len(bucket))
        comp_y.append(sum(computed_bucket) / len(computed_bucket))
        x.append(dist)
        s_x.append(_str_distance_bucket(max_dist)[i])

    # print(list(zip(x, y)))
    # print(list(zip(x, comp_y)))
    # print(x)
    # print(comp_y)

    # print('{', end='')
    # for ix, iy in zip(x, comp_y):
    #     print(f'({ix},{iy})', end='')
    # print('};', end='')

    data = [go.Scatter(
        x=s_x,
        y=y,
        name='measurements'
    ), go.Scatter(
        x=s_x,
        y=comp_y,
        name='computed values'
    )]

    score = sum([pow(abs(val1 - val2), 2) for val1, val2 in zip(y, comp_y)])
    print(score / len(measurement_raw_phili))
    layout = go.Layout(
        title=f'Phillippines log - Comparison for measure versus computed rssi - Score: {round(score, 3)}',
        xaxis=dict(title='Distance buckets'),
        yaxis=dict(title='RSSI')
    )
    fig = go.Figure(data=data, layout=layout)
    plotly.offline.plot(fig, filename='plots/scatter_bopl_plus_cvpl_vs_measurements.html', auto_open=False)


def build_average_rssi_diff_angle_buckets():
    measurements_phili = create_linkpairs('../gpslogs/phillippines_gps_rssi.txt', as_pair=True, remove_distance=True)
    measurements_rude = create_linkpairs('../gpslogs/rude_skov_gps_rssi.txt', as_pair=True, remove_distance=True)

    __, phili = parse_linkpairs_to_angle_bucket_sorted(measurements_phili)
    __, rude = parse_linkpairs_to_angle_bucket_sorted(measurements_rude)

    phili_y, phili_x, rude_y, rude_x = [], [], [], []
    for i, angle in enumerate(_gen_angle_buckets()):
        if len(phili[angle]) < 1:
            continue

        phili_y.append(sum(phili[angle]) / len(phili[angle]))
        phili_x.append(angle)

    for i, angle in enumerate(_gen_angle_buckets()):
        if len(rude[angle]) < 1:
            continue

        rude_y.append(sum(rude[angle]) / len(rude[angle]))
        rude_x.append(angle)

    print('phili')
    print('{', end='')
    for ix, iy in zip(phili_x, phili_y):
        print(f'({ix},{iy})', end='')
    print('};', end='')

    print()
    print('rude')
    print('{', end='')
    for ix, iy in zip(rude_x, rude_y):
        print(f'({ix},{iy})', end='')
    print('};', end='')


def plot_bopl_and_cvpl():
    links_above_80 = helpers.load_losmodel_computed_rssi_data('losmodel_link_data_above_80.json')
    link_less_5 = helpers.load_losmodel_computed_rssi_data('losmodel_link_data_less_5.json')
    sets = [
        ['building pct above 80', links_above_80, 'bopl', helpers.bopl],
        ['building pct below 5', link_less_5, 'cvpl', helpers.cvpl],
    ]

    max_dist = int(max([val.distance for val in link_less_5])) + 20
    divs = []
    score = 0
    for name, data, func_name, func in sets:
        prev = 0
        y, x, dist_x, traces = [], [], [], []
        for i, dist in enumerate(range(0, max_dist, 20)):
            bucket = [link.rssi for link in data if prev < link.distance <= dist]
            prev = dist

            if len(bucket) < 1:
                continue

            y.append(sum(bucket) / len(bucket))
            x.append(_str_distance_bucket(max_dist)[i])
            dist_x.append(dist)

        traces.append(go.Scatter(x=x, y=y, name=name))
        func_y = [26 - func(val) for val in dist_x]
        score = sum([pow(abs(val1 - val2), 2) for val1, val2 in zip(y, func_y)])

        traces.append(go.Scatter(
            x=x,
            y=func_y,
            name=func_name,
        ))
        fig = go.Figure(data=traces, layout={'title': f'Function = {func_name}   -   score = {round(score, 3)}'})
        divs.append(plotly.offline.plot(fig, output_type='div'))
        print(name)
        print('links = ', len(data))
        print('score = ', score / len(data))
        print(list(zip(dist_x, y)))
        print(y)
        print(dist_x)
        print()

    with open('constant_discovery_plot.html', 'w') as f:
        for div in divs:
            f.write(div)


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

        # compute_least_square(arg)

        # build scatter plot for least square regression for LoSModel V2
        # build_scatter_for_least_square_losmodel_v2(arg)

        # helpers.write_links_to_log('phili_links.csv', create_linkpairs(arg, as_pair=False), with_rssi=True)

        # build scatter plot of cvpl bople combo
        build_scatter_compare_field_cvpl_plus_bopl(arg)

        # average rssi diff for angle buckets
        # build_average_rssi_diff_angle_buckets()

        # plot the bopl and cvpl vs collected links
        # plot_bopl_and_cvpl()
