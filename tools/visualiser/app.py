from typing import Dict, Union

import threading
import random
import os
import uuid as uuids
import json

import flask
import plotly
import plotly.offline

UUID = str
ID = int
NodeDict = Dict[ID, Dict[str, float]]
LinkDict = Dict[ID, Dict[str, int]]
MapDict = Dict[UUID, Dict[str, Union[LinkDict, NodeDict]]]

lock = threading.Lock()
static_path = os.path.abspath('static')
template_path = os.path.abspath('templates')
app = flask.Flask(__name__, template_folder=template_path, static_folder=static_path)
maps: MapDict = {}
mb_access = 'pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A'


def generate_color() -> str:
    return f'rgb({random.choice(range(256))}, {random.choice(range(256))}, {random.choice(range(256))})'


# Sample data
maps['06e3c095-bac1-4964-b35e-d8e463094b85'] = {
    'links': {
        1: {
            'first': 1,
            'second': 2,
        },
        2: {
            'first': 2,
            'second': 3,
        },
        3: {
            'first': 1,
            'second': 3,
        }
    },
    'nodes': {
        1: {
            "lat": 14.629869,
            "lon": 121.096365,
        },
        2: {
            "lat": 14.632826,
            "lon": 121.097477,
        },
        3: {
            "lat": 14.635523,
            "lon": 121.088109,
        },
    }
}


@app.errorhandler(404)
def page_not_found(e):
    return flask.render_template('404.html', item='Page')


@app.route('/vis/')
def hello_world():
    return 'Hello, World!'


def compute_centroid(nodes: NodeDict) -> Dict[str, float]:
    lat, lon = 0.0, 0.0

    for node in nodes.values():
        lat += node['lat']
        lon += node['lon']

    lat = lat / len(nodes)
    lon = lon / len(nodes)

    return dict(lat=lat, lon=lon)


@app.route('/vis/maps/<uuid>')
def render_map(uuid):
    if uuid not in maps:
        return flask.render_template('404.html', item='Map'), 404

    # Create map
    data = []
    nodes = maps[uuid]['nodes']
    links = maps[uuid]['links']

    for i, link in links.items():
        data.append(plotly.graph_objs.Scattermapbox(
            lat=[nodes[link['first']]['lat'], nodes[link['second']]['lat']],
            lon=[nodes[link['first']]['lon'], nodes[link['second']]['lon']],
            opacity=0.5,
            mode='lines',
            hoverinfo='none',
            marker=dict(
                size=0.5,
                color='gray'
            )
        ))

    ids = []
    lats = []
    lons = []

    for i, node in nodes.items():
        ids.append(i)
        lats.append(node['lat'])
        lons.append(node['lon'])

    data.append(plotly.graph_objs.Scattermapbox(
        lat=lats,
        lon=lons,
        text=ids,
        mode='markers',
        marker=dict(
            size=5,
            color='rgb(65,105,225)'
        )
    ))

    centroid = compute_centroid(maps[uuid]['nodes'])
    layout = plotly.graph_objs.Layout(
        title=uuid,
        height=700,
        autosize=True,
        hovermode='closest',
        showlegend=False,
        dragmode='pan',
        mapbox=dict(
            accesstoken=mb_access,
            bearing=0,
            pitch=0,
            zoom=13,
            style='light',
            center=centroid
        ),
    )
    figure = dict(data=data, layout=layout)

    div = plotly.offline.plot(figure, output_type='div', include_plotlyjs='cdn', show_link=False)
    markup = flask.Markup(div)

    return flask.render_template('maps.html', uuid=uuid, map=markup)


@app.route('/vis/register-map')
def register_map():
    uuid = str(uuids.uuid4())
    maps[uuid] = {'links': {}, 'nodes': {}}
    return uuid


@app.route('/vis/add-nodes/<uuid>', methods=['POST'])
def add_nodes(uuid):
    recv = flask.request.json
    nodes = maps[uuid]['nodes']

    for node in recv:
        nodes[node['id']] = {}
        nodes[node['id']]['lat'] = node['lat']
        nodes[node['id']]['lon'] = node['lon']

    return 'Nodes added successfully'


@app.route('/vis/add-links/<uuid>', methods=['POST'])
def add_links(uuid):
    recv = flask.request.json
    links = maps[uuid]['links']

    for link in recv:
        links[link['id']] = {}
        links[link['id']]['first'] = link['first']
        links[link['id']]['second'] = link['second']

    return 'Links added successfully'


# @app.route('/vis/add-clusters', methods=['POST'])
# def add_clusters():
#     global nodes
#     clusters = flask.request.json
#
#     for cluster in clusters:
#         color = generate_color()
#         for node_id in cluster:
#             nodes[node_id]['color'] = color
#
#     return 'Clusters added successfully'

#
# @app.route('/vis/request-graph', methods=['POST'])
# def request_graph():
#     recv = flask.request.json
#     params = recv['params']
#     clusters = recv['clusters']
#     graph_nodes = nodes.copy()
#
#     for cluster in clusters:
#         color = generate_color()
#         for node_id in cluster:
#             nodes[node_id]['color'] = color
#
#     graph_id = params['id']
#     eps = params['eps']
#     minpts = params['minpts']
#     count = params['count']
#     title = f'eps: {eps}, minpts: {minpts}, count: {count}'
#
#     # nodes: dict, graph_id: int, token: str, path: str, title: str = 'Reachi'
#     executor.submit(create_graph.create_graph,
#                     nodes=graph_nodes, graph_id=graph_id, token=mb_access, path=os.path.abspath(graph_path),
#                     title=title)
#
#     return 'Graph request added successfully'
#
#
# plotly.io.orca.config.mapbox_access_token = mb_access
# executor = concurrent.futures.ProcessPoolExecutor()
#
# f = 'phillippines.json'
# with open(f, 'r') as json_data:
#     data = json.load(json_data)
#
#     nodes = {}
#
#     for i, locs in data.items():
#         loc = locs[len(locs) // 2]
#
#         nodes[i] = {}
#         nodes[i]['lat'] = loc["latitude"]
#         nodes[i]['lon'] = loc["longitude"]
#         nodes[i]['color'] = 'rgb(220, 20, 60)'
#
#     executor.submit(create_graph.create_graph,
#                     nodes=nodes, graph_id=21, token=mb_access, path=os.path.abspath(graph_path),
#                     title=f"Phillippines test data, {len(nodes)} nodes")

if __name__ == '__main__':
    app.run(host='0.0.0.0')
