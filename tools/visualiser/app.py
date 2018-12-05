import concurrent.futures

import flask
import random
import os

import plotly

import create_graph

static_path = os.path.abspath('static')
graph_path = os.path.abspath('graphs')
app = flask.Flask(__name__, template_folder=graph_path, static_folder=static_path)
executor = None
nodes = {}
mb_access = 'pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A'

if not os.path.exists(graph_path):
    os.mkdir(graph_path)


def generate_color() -> str:
    return f'rgb({random.choice(range(256))}, {random.choice(range(256))}, {random.choice(range(256))})'


@app.errorhandler(404)
def page_not_found(e):
    return flask.render_template('404.html', item='Page')


@app.route('/vis/')
def hello_world():
    return 'Hello, World!'


@app.route('/vis/graphs/<fig>')
def graphs(fig):
    graph = f'fig{fig}.html'
    if os.path.isfile(f'{graph_path}/{graph}'):
        return flask.render_template(graph)
    else:
        return flask.render_template('404.html', item='Graph'), 404


@app.route('/vis/add-nodes', methods=['POST'])
def add_nodes():
    global nodes
    recv = flask.request.json
    nodes = {}

    for node in recv:
        nodes[node['id']] = {}
        nodes[node['id']]['lat'] = node['lat']
        nodes[node['id']]['lon'] = node['lon']
        nodes[node['id']]['color'] = 'rgb(192, 192, 192)'

    return 'Nodes added successfully'


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


@app.route('/vis/request-graph', methods=['POST'])
def request_graph():
    recv = flask.request.json
    params = recv['params']
    clusters = recv['clusters']
    graph_nodes = nodes.copy()

    for cluster in clusters:
        color = generate_color()
        for node_id in cluster:
            nodes[node_id]['color'] = color

    graph_id = params['id']
    eps = params['eps']
    minpts = params['minpts']
    count = params['count']
    title = f'eps: {eps}, minpts: {minpts}, count: {count}'

    # nodes: dict, graph_id: int, token: str, path: str, title: str = 'Reachi'
    executor.submit(create_graph.create_graph,
                    nodes=graph_nodes, graph_id=graph_id, token=mb_access, path=os.path.abspath(graph_path),
                    title=title)

    return 'Graph request added successfully'


plotly.io.orca.config.mapbox_access_token = mb_access
executor = concurrent.futures.ProcessPoolExecutor()

if __name__ == '__main__':
    app.run(host='0.0.0.0')
