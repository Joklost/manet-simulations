import json
import random
from collections import deque

import flask
import dash
import dash_core_components
import dash_html_components
import pandas
import plotly
from dash.dependencies import Input, Output

server = flask.Flask(__name__)
app = dash.Dash(__name__, server=server)

nodes = {
    '0': {
        'lat': '57.00266813458001',
        'lon': '9.8929758',
    }
}


@server.route('/update', methods=['POST'])
def update():
    recv = flask.request.json
    nodes[recv['id']] = {}
    nodes[recv['id']]['lat'] = recv['lat']
    nodes[recv['id']]['lon'] = recv['lon']

    return "Update received succesfully."


app.layout = dash_html_components.Div(children=[
    dash_html_components.H1(children="Nodes"),

    dash_core_components.Graph(id='live-update-graph', animate=True),
    dash_core_components.Interval(id='interval-component', interval=1 * 1000, n_intervals=0),
])


@app.callback(Output('live-update-graph', 'figure'), [Input('interval-component', 'n_intervals')])
def update_graph_live(n):
    id = []
    lat = []
    lon = []
    for i, v in nodes.items():
        id.append(i)
        lat.append(v['lat'])
        lon.append(v['lon'])

    data = [
        plotly.graph_objs.Scattermapbox(
            lat=lat,
            lon=lon,
            text=id,
            mode='markers',
            marker=dict(
                size=5,
            )
        )
    ]

    layout = plotly.graph_objs.Layout(
        title='Reachi nodes',
        autosize=True,
        hovermode='closest',
        showlegend=False,
        mapbox=dict(
            accesstoken='pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A',
            bearing=0,
            pitch=0,
            zoom=3,
            style='light'
        ),
    )

    return dict(data=data, layout=layout)


    #
    # data = [
    #     dict(
    #         type='scattergeo',
    #         lat=lat,
    #         lon=lon,
    #         text=id,
    #         mode='markers',
    #         marker=dict(
    #             size=8,
    #         )
    #     )
    # ]
    #
    # layout = dict(
    #     width=1000,
    #     height=500,
    #     geo=dict(
    #         scope='world',
    #     ),
    # )

    #return dict(data=data, layout=layout)


if __name__ == '__main__':
    app.run_server(debug=True)
