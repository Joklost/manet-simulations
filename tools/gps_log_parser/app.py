import os
import json
import plotly.plotly as py
import plotly.offline
import plotly.graph_objs as go
import pandas as pd
import numpy as np
import math

MAX_ANGLE = 360


def make_plots():
    if not os.path.exists('data'):
        os.mkdir('data')

    data_files = os.listdir('data')
    plot_data = []

    for df in data_files:
        if df == 'nodes':
            continue

        with open(f'data/{df}', 'r') as fp:
            content = json.load(fp)

        # if df == 'nodes':
        #     plot_data.append(go.Bar(
        #         x=content['id'],
        #         y=content['count'],
        #         width=0.01,
        #         name=df
        #     ))
        # else:
        plot_data.append(go.Scatter(
            x=[i for i in range(0, MAX_ANGLE)],
            y=[content[f'{i}'] for i in range(0, MAX_ANGLE)],
            name=df
        ))

    layout = go.Layout(showlegend=True)
    fig = go.Figure(data=plot_data, layout=layout)

    if not os.path.exists('plots'):
        os.mkdir('plots')

    plotly.offline.plot(fig, filename='plots/model.html')


def parse_test_data_to_json(file: str):
    fn_parsed = f'parsed.{file}'
    with open(file, 'r') as old_f:
        with open(fn_parsed, 'w') as new_f:
            for line in old_f:
                new_f.writelines(f'{line[:-2]}\n')

    data = pd.read_csv(fn_parsed, sep=';', decimal=',')
    out = {}

    unique_timestamps = data['Time'].unique()
    for node in data['ID'].unique():
        df = data[data['ID'] == node]
        df.drop(df[(df['Latitude'] == 0.0) | (df['Longitude'] == 0.0)].index, inplace=True)
        out[f'{node}'] = []

        for _, e in df.iterrows():
            node_data = {
                'time': e['Time'],
                'lat': e['Latitude'],
                'lon': e['Longitude'],
                'rssi': {e[f'N{i}_ID']: {'rssi': e[f'N{i}_RSSI'], 'distance': e[f'N{i}_distance']} for i in range(1, 12)
                         if not math.isnan(e[f'N{i}_ID'])}
            }
            out[f'{node}'].append(node_data)
    # for time in data['Time'].unique():
    #     out[f'{time}'] = []
    #     df_at_t = data[data['Time'] == time]
    #     for _, e in df_at_t.iterrows():
    #         out[f'{time}'].append({
    #             'id': e['ID'],
    #             'lat': e['Latitude'],
    #             'lon': e['Longitude'],
    #             'rssi': {}
    #         })
    #         size = len(out[f'{time}'])
    #
    #         for i in range(1, 12):
    #             node_id = e[f'N{i}_ID']
    #             if math.isnan(node_id):
    #                 continue
    #
    #             out[f'{time}'][size - 1]['rssi'][node_id] = {
    #                 'rssi': e[f'N{i}_RSSI'],
    #                 'distance': e[f'N{i}_distance']
    #             }

    f_name, _ = os.path.splitext(file)
    with open(f'{f_name}.json', 'w') as json_f:
        json.dump(out, json_f)


def parse_test_data_to_plots(file: str):
    with open(file, 'r') as fp:
        data = json.load(fp)

    curr_max = 0
    best_timestamp = ''
    for i, e in data.items():
        has_gps = True
        for node in e:
            if node['lat'] == 0.0 or node['lon'] == 0.0:
                has_gps = False

        if has_gps:
            sum = 0
            for node in e:
                sum += len(node['rssi'])

            if sum > curr_max:
                curr_max = sum
                best_timestamp = i

    nodes = data[best_timestamp]
    print(json.dumps(nodes))


def gen_nodes(time):
    out = []
    with open('data.json', 'r') as f:
        data = json.load(f)

    for i, e in data.items():
        for d in e:
            if d['time'] == time:
                out.append({
                    'node_id': i,
                    'lat': d['lat'],
                    'lon': d['lon']
                })

    with open('data/nodes', 'w') as f:
        f.writelines('# node id; latitude; longitude')
        for node in out:
            f.writelines(f'{node["node_id"]};{node["lat"]};{node["lon"]}\n')


if __name__ == '__main__':
    logs_path = '../../src/logs'
    logview_path = f'{logs_path}/logview'
    parsed_path = f'{logs_path}/parsed'

    if not os.path.exists(logview_path):
        os.mkdir(logview_path)

    if not os.path.exists(parsed_path):
        os.mkdir(parsed_path)

    for log in [i for i in os.listdir(logview_path) if not i.startswith('fixed_')]:
        with open(f'{logview_path}/{log}', 'r') as f:
            with open(f'{logview_path}/fixed_{log}', 'w') as ff:
                for line in f:
                    ff.writelines(f'{line[:-2]}\n')

    for log in [i for i in os.listdir(logview_path) if i.startswith('fixed_')]:
        with open(f'{logview_path}/{log}', 'r') as f:
            df = pd.read_csv(f, sep=';', decimal=',')

        out_df = df
        min_time = df['Time'].min()

        for i, e in out_df.iterrows():
            out_df.iloc[i].replace({'Time': e['Time'] - min_time}, inplace=True)

        print(out_df['Time'])

        # for time in df['Time'].unique():
        #     time_sorted = df[df['Time'] == time]
            # for i, e in time_sorted.iterrows():






# make_plots()
# parse_test_data_to_json('data.csv')
# gen_nodes(1510737860)
# with open('data/measured', 'w') as f:
#     with open('data.json', 'r') as fp:
#         data = json.load(fp)
#
#     out = []
#     for i, e in data.items():
#         for d in e:
#             if d['time'] == 1510737860:
#                 TODO: NO! skal finde for hvert link ikke node
# out.append({
#     'node_id': i,
#     'rssi': d['rssi']
# })
# parse_test_data_to_plots('data.json')
