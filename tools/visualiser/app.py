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


def create_graph(node_data):
    id = []
    lat = []
    lon = []
    for i, v in node_data.items():
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

    if lat and lon:
        center = dict(
            lat=float(lat[0]),
            lon=float(lon[0])
        )
    else:
        center = dict(
            lat=57.00266813458001,
            lon=9.8929758
        )

    layout = plotly.graph_objs.Layout(
        title='Reachi nodes',
        height=800,
        autosize=True,
        hovermode='closest',
        showlegend=False,
        dragmode='pan',
        mapbox=dict(
            accesstoken='pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A',
            bearing=0,
            pitch=0,
            zoom=8,
            style='light',
            center=center
        ),
    )

    return dict(data=data, layout=layout)


@server.route('/update', methods=['POST'])
def update():
    recv = flask.request.json
    nodes[recv['id']] = {}
    nodes[recv['id']]['lat'] = recv['lat']
    nodes[recv['id']]['lon'] = recv['lon']

    return "Update received succesfully."


app.layout = dash_html_components.Div(children=[
    dash_core_components.Graph(id='live-update-graph', figure=create_graph(nodes)),
    dash_core_components.Interval(id='interval-component', interval=1000, n_intervals=0),
])


@app.callback(Output('live-update-graph', 'figure'),
              [Input('interval-component', 'n_intervals'), Input('live-update-graph', 'relayoutData')])
def update_graph_live(n, relayout):
    figure = create_graph(nodes)
    print(relayout)
    if relayout and 'mapbox.zoom' in relayout:
        figure['layout']['mapbox']['zoom'] = relayout['mapbox.zoom']

    if relayout and 'mapbox.center' in relayout:
        figure['layout']['mapbox']['center'] = dict(
            lat=relayout['mapbox.center']['lat'],
            lon=relayout['mapbox.center']['lon']
        )

    return figure


if __name__ == '__main__':
    app.run_server(debug=True)
