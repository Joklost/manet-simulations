

//const key = 'AIzaSyDVGtIxq1ZBrbYhW4mJs5WU9UgQXIJGhx0';
const mappa = new Mappa('Leaflet');
let map_canvas;
let map;
let slider_with = 590;
let time_slider;
let glob_time = 0;
let time_slider_active = false;
let speed_slider;
let current_data = null;
let pause = true;

const options = {
    lat: 0,
    lng: 0,
    zoom: 4,
    style: "http://{s}.tile.osm.org/{z}/{x}/{y}.png"
}
$(document).ready(function () {
    // listen for changes
    $("#type").change(function (e) {
        update_settings(this);
    });
    $.getJSON("/models/", function (data) {
        if (data === null || data.hasOwnProperty("error") || !data.hasOwnProperty("models"))
        {
            if (data === null || !data.hasOwnProperty("models"))
            {
                handle_error("No/Wrong data recieved!");
                console.log(data);
            } else
                handle_error(data.error);
        } else
        {
            add_models(data.models);
        }
    });
    update_settings($("#type"));

    // initialize the map
    let show_map = function (p) {
        p.setup = function () {
            map_canvas = p.createCanvas(p.windowWidth, 640);
            map = mappa.tileMap(options);
            map.overlay(map_canvas)
            map_canvas.fill(200, 100, 100);

            // Only redraw the point when the map changes and not every frame.
            map.onChange(p.drawPoint);
        };
        p.drawPoint = function () {
        };
        const diam = 20;
        const to_diam = 5;
        p.draw = function () {
            if (current_data !== null)
            {
                p.clear();
                var speed = $("#speed_slider").val();
                if (speed < 0)
                    speed = 1 / Math.abs(speed);
                else
                    speed = 1 + Math.abs(speed);
                if (!time_slider_active && !pause)
                {

                    glob_time = glob_time + (1000 / p.frameRate() * speed);
                    $("#time_slider").val(glob_time);
                } else if(time_slider_active)
                {
                    glob_time = parseFloat($("#time_slider").val());
                }
                $("#time_val").html(glob_time);
                $("#speed_val").html("x " + speed);

                // draw edges in the bottom
                for (var id in current_data.nodes)
                {
                    const data = node_data(id, glob_time);
                    if (data === null)
                        continue;      
                    const dot = map.latLngToPixel(data);
                    var edges = node_edges(id, glob_time);
                    for(var eid in edges)
                    {      
                        const odata = node_data(edges[eid].dest, glob_time);
                        var odot = map.latLngToPixel(odata);
                        show_edge(p, edges[eid], data, odata, dot, odot);
                        p.line(dot.x, dot.y, odot.x, odot.y);
                        // add some top here!
                    }
                }
                
                var some = false;

                for (var id in current_data.nodes)
                {
                    const data = node_data(id, glob_time);
                    if (data === null)
                        continue;
                    some = true;
                    const dot = map.latLngToPixel(data);
                    show_node(p, data, dot);
                    p.ellipse(dot.x, dot.y, diam, diam);                    
                    p.text(id, dot.x-(diam/3), dot.y+(diam/4));
                }
                
                if (!some)
                {
                    pause = true;
                }
            }
        };
        p.mouseClicked = function ()
        {
            if (current_data !== null)
            {
                for (var id in current_data.nodes)
                {
                    const data = node_data(id, glob_time);
                    const dot = map.latLngToPixel(data);
                    if (Math.sqrt(Math.pow(p.mouseX - dot.x, 2) + Math.pow(p.mouseY - dot.y, 2)) < diam)
                    {
                        console.log(current_data.nodes[id]);
                    }
                }
            }
        };
        p.keyPressed = function () {
            if (p.keyCode === p.LEFT_ARROW) {
                pause = true;
                glob_time -= 1;
            } else if (p.keyCode === p.RIGHT_ARROW) {
                pause = true;
                glob_time += 1;
            }
        }
    };
    new p5(show_map, window.document.getElementById("visualization"));

    // add controlls

    $("#time_slider").mousedown(function () {
        time_slider_active = true;
    });

    $("#time_slider").change(function () {
        time_slider_active = false;
    });
    $("#play").click(function () {
        pause = false;
    });
    $("#pause").click(function () {
        pause = true;
    });
    $("#stop").click(function () {
        pause = true;
        if (current_data)
            $("#time_slider").val(current_data.first_time);
    });
});

function show_node(canvas, data, coordinates)
{
    canvas.strokeWeight(1);
    canvas.stroke(0, 0, 0);
}

function show_edge(canvas, edge, from, to, from_coord, to_coord)
{
    var dist = map.map.distance(from, to);
    canvas.strokeWeight(3);
    var v = Math.max(Math.min(255, dist/20));
    canvas.stroke(v, v, v);
}

function extrapolate_node(pre, post, delta)
{
    return pre;
}

function extrapolate_edge(pre, post, delta)
{
    return pre;
}

function node_data(id, time)
{
    if (current_data.nodes[id][0].timestamp > time ||
            current_data.nodes[id][current_data.nodes[id].length - 1].timestamp < time)
        return null;
    var post = 0;
    while (current_data.nodes[id][post].timestamp < time)
        ++post;

    if (post === 0)
    {
        return current_data.nodes[id][post];
    } 
    else
    {
        var part = current_data.nodes[id][post].timestamp - current_data.nodes[id][post - 1].timestamp;
        part = (time - current_data.nodes[id][post - 1].timestamp) / part;
        var pre = $.extend(true, {}, current_data.nodes[id][post - 1]);
        var postd = current_data.nodes[id][post];
        pre.lat += (postd.lat - pre.lat)*part;
        pre.lng += (postd.lng - pre.lng)*part;    
        return extrapolate_node(pre, postd, part);
    }
}

function node_edges(id, time)
{
    var res = [];
    if (current_data.edges[id] == null) return res;
        
    for(var eid in current_data.edges[id])
    {
        if (current_data.edges[id][eid][0].timestamp > time ||
                current_data.edges[id][eid][current_data.edges[id][eid].length - 1].timestamp < time)
            continue;
        var post = 0;
        while(current_data.edges[id][eid][post].timestamp < time)
            ++post;
        if (post === 0)
            res.push(current_data.edges[id][eid][post]);
        else
        {
            var part = current_data.edges[id][eid][post].timestamp - current_data.edges[id][eid][post-1].timestamp; 
                part = (time - current_data.edges[id][eid][post - 1].timestamp) / part;
                res.push(extrapolate_edge($.extend(true, {}, current_data.edges[id][eid][post - 1]), current_data.nodes[id][post], part));
        }
    }
    return res;
}

function update_settings(el)
{
    var value = $(el).val();
    var handler = null;
    $(".settings").hide();
    if (value === 'static')
    {
        $("#static_topology.settings").show();
        handler = execution_handler;
    } else if (value === 'gps')
    {
        $("#gps_topology.settings").show();
        handler = execution_handler;
    } else if (value === 'load')
    {
        handle_error("Not yet implemented");
        handler = handle_load;
    } else
    {
        handle_error("Unknown Simulation-type selected: " + val);
        handler = handle_error;
    }
    $("form#settings").submit(function (e) {
        e.preventDefault();
        handler(this);
    });
}

function handle_error(message)
{
    console.log(message);
    $("#settings #error.setting").html("<p class='error'>" + message + "</p>");
}
function execution_handler(element)
{
    var fd = new FormData(element);
    $.ajax({
        url: "/execute/",
        type: "POST",
        data: fd,
        processData: false,
        contentType: false,
        enctype: 'multipart/form-data',
        success: function (data, textStatus, jqXHR) {
            data = $.parseJSON(data);
            if (data === null)
            {
                handle_error("An unknown response from the server");
            } else if (data.hasOwnProperty("error"))
            {
                handle_error(data.error);
            } else
            {
                show_simulation(data);
            }
        }
    });
}

function handle_load()
{
    alert("Loading is not yet implemented");
}

function add_models(models)
{
    var data = "";
    for (var i in models)
    {
        var first = (data.length === 0);
        data += "<option value='" + models[i] + "' ";
        if (first)
            data += "selected=selected "
        data += ">" + models[i] + "</option>";
    }
    $("#model").html(data);
}


// lets do the actual display!
function show_simulation(data)
{
    if(data === {})
        return;
    $("#speed_slider").attr('min', -100);
    $("#speed_slider").attr('max', 99);
    $("#speed_slider").val(0);
    $("#time_slider").attr('min', data.first_time);
    $("#time_slider").attr('max', data.last_time);
    $("#time_slider").val(data.first_time);
    glob_time = data.first_time;
    current_data = data;

    var bounds = [[data.min_lat, data.min_lng], [data.max_lat, data.max_lng]];
    map.map.fitBounds(bounds);
}
