import os
import uuid as uuids
from typing import Dict, Union
import json
from backend import engine

import flask
import plotly
import plotly.offline

UUID = str
ID = int
NodeDict = Dict[ID, Dict[str, float]]
LinkDict = Dict[ID, Dict[str, int]]
MapDict = Dict[UUID, Dict[str, Union[LinkDict, NodeDict]]]

static_path = os.path.abspath('static')
template_path = os.path.abspath('templates')
app = flask.Flask(__name__, template_folder=template_path, static_folder=static_path)
maps: MapDict = {}
mb_access = 'pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A'

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


@app.route('/')
def index():
    return flask.redirect('/vis/')


@app.route('/animated/')
def simulate():
    return flask.render_template('animated.html')


@app.route('/models/', methods=['GET'])
def models():
    return json.dumps(engine.list_models())


@app.route('/execute/', methods=['POST'])
def execute():
    result = engine.execute(json.loads(flask.request.data))
    return json.dumps(result)


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
    maps[uuid]['nodes'] = {}
    nodes = maps[uuid]['nodes']

    for node in recv:
        nodes[node['id']] = {}
        nodes[node['id']]['lat'] = node['lat']
        nodes[node['id']]['lon'] = node['lon']

    return 'Nodes added successfully'


@app.route('/vis/add-links/<uuid>', methods=['POST'])
def add_links(uuid):
    recv = flask.request.json
    maps[uuid]['links'] = {}
    links = maps[uuid]['links']

    for link in recv:
        links[link['id']] = {}
        links[link['id']]['first'] = link['first']
        links[link['id']]['second'] = link['second']

    return 'Links added successfully'


if __name__ == '__main__':
    app.run(host='0.0.0.0')
