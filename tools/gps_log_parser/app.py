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

    cols_ordering = ['ID', 'Latitude', 'Longitude', 'Time', 'N0_ID', 'N0_RSSI', 'N1_ID', 'N1_RSSI'
        , 'N2_ID', 'N2_RSSI', 'N3_ID', 'N3_RSSI', 'N4_ID',
                     'N4_RSSI', 'N5_ID', 'N5_RSSI', 'N6_ID', 'N6_RSSI',
                     'N7_ID', 'N7_RSSI', 'N8_ID', 'N8_RSSI', 'N9_ID', 'N9_RSSI',
                     'N10_ID', 'N10_RSSI', 'N11_ID', 'N11_RSSI']

    for log in [i for i in os.listdir(logview_path) if i.startswith('fixed_')]:
        with open(f'{logview_path}/{log}', 'r') as f:
            df = pd.read_csv(f, sep=';', decimal=',')

        out_df = df.drop(
            columns=['UID', 'N0_distance', 'N1_distance', 'N2_distance', 'N3_distance', 'N4_distance', 'N5_distance',
                     'N6_distance', 'N7_distance', 'N8_distance', 'N9_distance', 'N10_distance', 'N11_distance'])
        out_df = out_df[cols_ordering]

        # fix Timestamp
        min_time = df['Time'].min()
        for i, e in out_df.iterrows():
            out_df.at[i, 'Time'] = e['Time'] - min_time

        # fix RSSI
        for i, e in df.iterrows():
            time = e['Time']
            curr_row_id = e['ID']
            for id_count in range(0, 12):
                curr_col_id = f'N{id_count}_ID'
                if math.isnan(e[curr_col_id]):
                    continue

                rssi_sum = e[f'N{id_count}_RSSI']
                match = df.loc[(df['Time'] == time) & (df['ID'] == e[curr_col_id])]
                if not match.empty:
                    index = df.loc[(df['Time'] == time) & (df['ID'] == e[curr_col_id])].index[0] - 1
                    rssi_sum += match[f'N{id_count}_RSSI']
                    rssi_sum = rssi_sum / 2
                    out_df.at[index, f'N{id_count}_RSSI'] = rssi_sum

                out_df.at[i, f'N{id_count}_RSSI'] = rssi_sum

        out_df = out_df.astype({'Time': float})

        with open(f'{parsed_path}/{log[6:]}', 'w') as f:
            for i, e in out_df.iterrows():
                text = ''
                for col_name in cols_ordering:
                    if not math.isnan(e[col_name]):
                        if 'ID' in col_name:
                            text += f'{int(out_df.at[i, col_name])},'
                        else:
                            text += f'{out_df.at[i, col_name]},'
                f.writelines(f'{text[:-1]}\n')
