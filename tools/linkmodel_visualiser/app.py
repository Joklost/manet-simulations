import os
import json
import plotly.plotly as py
import plotly.offline
import plotly.graph_objs as go


def main():
    data_files = os.listdir('data')
    plot_data = []

    for df in data_files:
        content = json.load(f'data/{df}')

        if df == 'nodes':
            plot_data.append(go.Bar(
                x=content['id'],
                y=content['count'],
                width=0.01,
                name=df
            ))
        else:
            plot_data.append(go.Scatter(
                x=[],
                y=[],
                name=df
            ))

    plotly.offline.plot(plot_data, filename='plots/model.html')


if __name__ == '__main__':
    main()
