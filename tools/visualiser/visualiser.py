import os
import random
from concurrent.futures import ProcessPoolExecutor

import flask
import dash
import dash_core_components
import dash_html_components
import plotly
import plotly.io
from dash.dependencies import Input, Output

server = flask.Flask(__name__)
app = dash.Dash(__name__, server=server)

nodes = {}

DEFAULT_COLOR = 'rgb(17, 157, 255)'
if not os.path.exists('images'):
    os.mkdir('images')

plotly.io.orca.config.mapbox_access_token = 'pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A'


def create_graph(node_data, title: str = 'Reachi'):
    clusters = {}

    for i, v in node_data.items():
        if v['color'] not in clusters:
            clusters[v['color']] = {
                'id': [],
                'lat': [],
                'lon': [],
            }

        clusters[v['color']]['id'].append(i)
        clusters[v['color']]['lat'].append(v['lat'])
        clusters[v['color']]['lon'].append(v['lon'])

    data = []

    for color, cluster in clusters.items():
        data.append(plotly.graph_objs.Scattermapbox(
            lat=cluster['lat'],
            lon=cluster['lon'],
            text=cluster['id'],
            mode='markers',
            marker=dict(
                size=5,
                color=color
            )
        ))

    if not data:
        data.append(plotly.graph_objs.Scattermapbox(
            lat=[],
            lon=[],
            text=[],
            mode='markers',
            marker=dict(
                size=5,
            )
        ))

    # data = [
    #     plotly.graph_objs.Scattermapbox(
    #         lat=lat,
    #         lon=lon,
    #         text=id,
    #         mode='markers',
    #         # xaxis=dict(fixedrange=True),
    #         # yaxis=dict(fixedrange=True),
    #         marker=dict(
    #             size=5,
    #         )
    #     )
    # ]

    center = dict(
        lon=8.725199732511442,
        lat=56.81765649206909
    )

    layout = plotly.graph_objs.Layout(
        title=title,
        height=700,
        autosize=True,
        hovermode='closest',
        showlegend=False,
        dragmode='pan',
        mapbox=dict(
            accesstoken='pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A',
            bearing=0,
            pitch=0,
            zoom=9.5,
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


@server.route('/updatechunk', methods=['POST'])
def updatechunk():
    recv = flask.request.json

    for node in recv:
        nodes[node['id']] = {}
        nodes[node['id']]['lat'] = node['lat']
        nodes[node['id']]['lon'] = node['lon']
        nodes[node['id']]['color'] = 'rgb(17, 157, 255)'

    return "Chunk update received successfully."


#
# colour = 0
# colours = [
#     'rgb(255, 157, 255)',
#     'rgb(255, 157, 1)',
#     'rgb(1, 157, 1)',
#     'rgb(255, 1, 1)',
#     'rgb(255, 1, 255)',
# ]

def generate_color() -> str:
    return f'rgb({random.choice(range(256))}, {random.choice(range(256))}, {random.choice(range(256))})'


@server.route('/colornodes', methods=['POST'])
def colornodes():
    global nodes
    recv = flask.request.json
    for cluster in recv:
        color = generate_color()
        for node in cluster:
            nodes[node]['color'] = color

    return "Colour update received successfully."


if __name__ == '__main__':
    executor = ProcessPoolExecutor()


def save_graph(params, nodes):
    id = params['id']
    print(f'Saving fig{id}.png')
    eps = params['eps']
    minpts = params['minpts']
    t_eps = params['t_eps']
    count = params['count']

    figure = create_graph(nodes, f'eps: {eps}, minpts: {minpts}, t_eps: {t_eps}, count: {count}')
    plotly.io.write_image(figure, f'images/fig{id}.png')


@server.route('/save', methods=['POST'])
def save():
    recv = flask.request.json

    if __name__ == '__main__':
        executor.submit(save_graph, recv, nodes)

    return "Image save request added."


@server.route('/clear')
def clear():
    global nodes
    nodes = {}
    return "Nodes cleared successfully."


app.layout = dash_html_components.Div(children=[
    dash_core_components.Graph(id='live-update-graph', figure=create_graph(nodes)),
    dash_core_components.Interval(id='interval-component', interval=2000),

    dash_core_components.Slider(
        id='zoom-slider',
        min=1,
        max=10,
        value=10,
        marks={str(i): str(i) for i in range(0, 11)}
    ),
])


@app.callback(Output('live-update-graph', 'figure'),
              [Input('interval-component', 'n_intervals'),
               Input('live-update-graph', 'relayoutData'),
               Input('zoom-slider', 'value')])
def update_graph_live(n, relayout, value):
    figure = create_graph(nodes)

    if value is not None:
        figure['layout']['mapbox']['zoom'] = value

    if relayout and 'mapbox.center' in relayout:
        figure['layout']['mapbox']['center'] = dict(
            lat=relayout['mapbox.center']['lat'],
            lon=relayout['mapbox.center']['lon']
        )

    return figure


if __name__ == '__main__':
    app.run_server()
    executor.shutdown()
