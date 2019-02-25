import plotly
import plotly.offline


def compute_centroid(nodes: dict):
    lat = 0
    lon = 0

    for node in nodes.values():
        lat += node['lat']
        lon += node['lon']

    lat = lat / len(nodes)
    lon = lon / len(nodes)

    return lat, lon


def create_graph(nodes: dict, graph_id: int, token: str, path: str, title: str = 'Reachi'):

    clusters = {}

    for i, v in nodes.items():
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

    centroid = compute_centroid(nodes)
    center = dict(
        lat=centroid[0],
        lon=centroid[1],
    )

    layout = plotly.graph_objs.Layout(
        title=title,
        height=700,
        autosize=True,
        hovermode='closest',
        showlegend=False,
        dragmode='pan',
        mapbox=dict(
            accesstoken=token,
            bearing=0,
            pitch=0,
            zoom=9.5,
            style='light',
            center=center
        ),
    )

    figure = dict(data=data, layout=layout)
    filename = f'{path}/fig{graph_id:04}.html'
    print(f'Saving graph as "{filename}"')

    plotly.offline.plot(figure, filename=filename, auto_open=False, include_plotlyjs='cdn', show_link=False)
