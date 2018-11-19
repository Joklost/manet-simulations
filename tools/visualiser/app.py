import flask
import dash
import dash_core_components
import dash_html_components
import plotly
from dash.dependencies import Input, Output

server = flask.Flask(__name__)
app = dash.Dash(__name__, server=server)

nodes = {}


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
            # xaxis=dict(fixedrange=True),
            # yaxis=dict(fixedrange=True),
            marker=dict(
                size=5,
            )
        )
    ]

    center = dict(
        lon=8.725199732511442,
        lat=56.81765649206909
    )

    layout = plotly.graph_objs.Layout(
        height=700,
        autosize=True,
        hovermode='closest',
        showlegend=False,
        dragmode='pan',
        mapbox=dict(
            accesstoken='pk.eyJ1Ijoiam9rbG9zdCIsImEiOiJjam5kN2V1d3gyNXpvM3FyZm01aGE5emRlIn0.xpGYl9Ayd1FmDS2HS-Uf1A',
            bearing=0,
            pitch=0,
            zoom=10.50,
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

    return "Chunk update received successfully."


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
    print(relayout)
    print(value)

    if value is not None:
        figure['layout']['mapbox']['zoom'] = value

    if relayout and 'mapbox.center' in relayout:
        figure['layout']['mapbox']['center'] = dict(
            lat=relayout['mapbox.center']['lat'],
            lon=relayout['mapbox.center']['lon']
        )

    return figure


if __name__ == '__main__':
    app.run_server(debug=True)
