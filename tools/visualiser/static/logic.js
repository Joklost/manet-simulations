

const mappa = new Mappa('Leaflet');
let map_canvas;
let map;
let glob_time = 0;
let time_slider_active = false;
let current_data = null;
let pause = true;
let settings = {
    "style": {
        "node_style":
                "canvas.strokeWeight(1);\n" +
                "canvas.stroke(0, 0, 0);\n",
        "node_extrapolation": "",
        "edge_style": "var dist = map.map.distance(from, to);\n" +
                "canvas.strokeWeight(3);\n" +
                "if(dist < 200)\n" +
                "{\n" +
                "    canvas.stroke(255, 0, 0,255);\n" +
                "}\n" +
                "else\n" +
                "{\n" +
                "    canvas.stroke(0,0,0,0);\n" +
                "}\n",
        "edge_extrapolation": "",
    },
    "simulation_type": {},
    "simulation_data": []
};
let style_mode = "";
let code_mirror;


const options = {
    lat: 0,
    lng: 0,
    zoom: 4,
    style: "http://{s}.tile.osm.org/{z}/{x}/{y}.png"
}

$(document).ready(function () {
    // listen for changes
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

    // initialize the map
    let show_map = function (p) {
        p.setup = function () {
            map_canvas = p.createCanvas(p.windowWidth, p.windowHeight - 30);
            map = mappa.tileMap(options);
            map.overlay(map_canvas)
            map_canvas.fill(200, 100, 100);

            // Only redraw the point when the map changes and not every frame.
            map.onChange(p.drawPoint);
            p.frameRate(24);
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

                    glob_time = glob_time + ((1000 / p.frameRate()) * speed);
                    $("#time_slider").val(glob_time);
                } else if (time_slider_active)
                {
                    glob_time = parseFloat($("#time_slider").val());
                } else if (p.keyIsDown(p.LEFT_ARROW))
                {
                    glob_time -= (speed / p.frameRate());
                } else if (p.keyIsDown(p.RIGHT_ARROW))
                {
                    glob_time += (speed / p.frameRate());
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
                    for (var eid in edges)
                    {
                        const odata = node_data(edges[eid].dest, glob_time);
                        var odot = map.latLngToPixel(odata);
                        _show_edge(p, edges[eid], data, odata, dot, odot);
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
                    _show_node(p, data, dot);
                    p.ellipse(dot.x, dot.y, diam, diam);
                    p.text(id, dot.x - (diam / 3), dot.y + (diam / 4));
                }

                if (!some)
                {
                    $("#pause").click();
                }
            }
        };
        /*p.mouseClicked = function ()
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
         };*/
        p.keyPressed = function () {
            var speed = $("#speed_slider").val();
            if (speed < 0)
                speed = 1 / Math.abs(speed);
            else
                speed = 1 + Math.abs(speed);
            speed /= 24;
            if (p.keyCode === p.LEFT_ARROW) {
                $("#pause").click();
                glob_time -= speed;
            } else if (p.keyCode === p.RIGHT_ARROW) {
                $("#pause").click();
                glob_time += speed;
            } else if (p.keyCode === 32) // space
            {
                if (pause)
                    $("#play").click();
                else
                    $("#pause").click();
            }
        };
        p.windowResized = function() {
            $(".leaflet-container").width(p.windowWidth).height(p.windowHeight - 30);
            p.resizeCanvas(p.windowWidth, p.windowHeight - 30, true);
            //map = mappa.tileMap(options);
            //map.overlay(map_canvas)            
        };
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
        if (current_data !== null)
        {
            pause = false;
            $("#play").attr('class', 'active');
            $("#pause").attr('class', '');
            $("#stop").attr('class', '');
        }
    });
    $("#pause").click(function () {
        if (current_data !== null)
        {
            pause = true;
            $("#play").attr('class', '');
            $("#pause").attr('class', 'active');
            $("#stop").attr('class', '');
        }
    });
    $("#stop").click(function () {
        pause = true;
        if (current_data)
        {
            $("#time_slider").val(current_data.first_time);
            glob_time = current_data.first_time;
        }
        $("#play").attr('class', '');
        $("#pause").attr('class', '');
        $("#stop").attr('class', 'active');
    });
    $(".top_menu").hover(function () {
        $("ul", this).slideDown(100);
            var width = $(this).width();
            $('li', this).each(function(){
               width = Math.max($(this).width(), width); 
            });
            $('li', this).each(function(){
               $(this).width(width); 
            });
    }, function () {
        $("ul", this).stop().slideUp(100);
    });
    $("#stop").attr('class', 'active');
    $(".shade").click(function () {
        $(this).parent().hide(200);
    });
    const cmopts = {
        lineNumbers: true,
        mode: "javascript",
        style: "darcula",
        styleActiveLine: true,
        matchBrackets: true
    };
    code_mirror = CodeMirror.fromTextArea(window.document.getElementById("style_textarea"), cmopts);

    $("#apply").click(function (e) {
        e.preventDefault();
        $("#style_input").hide(200);
        settings.style[style_mode] = code_mirror.getValue();
        set_style(style_mode);
        style_mode = "";
    });

    set_styles();

    $("#style_file").change(function () {
        load_file(this, function (e) {
            var lines = e.target.result;
            var data = JSON.parse(lines);
            settings.style = data;
            set_styles();
        });
    });
    $("#settings_file").change(function () {
        load_file(this, function (e) {
            var lines = e.target.result;
            var data = JSON.parse(lines);
            settings = data;
            set_styles();
        });
    });
    $("#simulation_file").change(function () {
        load_file(this, function (e) {
            var lines = e.target.result;
            var data = JSON.parse(lines);
            show_simulation(data);
        });
    });
    $("#gps_log").change(function () {
        load_file(this, function (e) {
            var lines = e.target.result;
            // this could be done in pure javascript, no need to call backend, really.
            settings.simulation_type.gps_data = e.target.result;
            settings.simulation_type.type = "log";
            execution_handler();
        });
    });
});

function load_file(element, handler)
{
    if (!element.files[0]) {
        alert("Could not load the file");
    } else {
        var file = element.files[0];
        var fr = new FileReader();
        fr.onload = handler;
        fr.readAsText(file);
    }
}

function set_styles()
{
    set_style('node_style');
    set_style('node_extrapolation');
    set_style('edge_extrapolation');
    set_style('edge_style');
}

function set_style(style_mode)
{
    if (style_mode === 'node_style')
    {
        eval("_show_node = function(canvas, node, point){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'node_extrapolation')
    {
        eval("_extrapolate_node = function(pre, post, delta){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'edge_style')
    {
        eval("_show_edge = function(canvas, edge, from, to, from_point, to_point){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'edge_extrapolation')
    {
        eval("_extrapolate_edge = function(pre, post, delta){" + settings.style[style_mode] + "}");
    } else
    {
        alert("Uknown style setting " + style_mode);
    }
}


var _show_node = function (canvas, node, point) {}
var _show_edge = function (canvas, edge, from, to, from_point, to_point) {}
var _extrapolate_node = function (pre, post, delta) {}
var _extrapolate_edge = function (pre, post, delta) {}

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
    } else
    {
        var part = current_data.nodes[id][post].timestamp - current_data.nodes[id][post - 1].timestamp;
        part = (time - current_data.nodes[id][post - 1].timestamp) / part;
        var pre = $.extend(true, {}, current_data.nodes[id][post - 1]);
        var postd = current_data.nodes[id][post];
        pre.lat += (postd.lat - pre.lat) * part;
        pre.lng += (postd.lng - pre.lng) * part;
        _extrapolate_node(pre, postd, part);
        return pre;
    }
}

function node_edges(id, time)
{
    var res = [];
    if (current_data.edges[id] == null)
        return res;

    for (var eid in current_data.edges[id])
    {
        if (current_data.edges[id][eid][0].timestamp > time ||
                current_data.edges[id][eid][current_data.edges[id][eid].length - 1].timestamp < time)
            continue;
        var post = 0;
        while (current_data.edges[id][eid][post].timestamp < time)
            ++post;
        if (post === 0)
            res.push(current_data.edges[id][eid][post]);
        else
        {
            var part = current_data.edges[id][eid][post].timestamp - current_data.edges[id][eid][post - 1].timestamp;
            part = (time - current_data.edges[id][eid][post - 1].timestamp) / part;
            var pre = $.extend(true, {}, current_data.edges[id][eid][post - 1]);
            _extrapolate_edge(pre, current_data.nodes[id][post], part)
            res.push(pre);
        }
    }
    return res;
}

function update_settings(value)
{
    $("#type").val(value);
    $(".settings").hide();
    if (value === 'static')
    {
        $("#static_topology.settings").show();
    } 
    else if (value === 'gps')
    {
        $("#gps_topology.settings").show();
    } else
    {
        handle_error("Unknown Simulation-type selected: " + value);
        return;
    }
    $("#settings").show(200);
    $("#settings form").submit(function (e) {
        e.preventDefault();
        settings.simulation_type = {};
        settings.simulation_type.model = $("#model").val();
        settings.simulation_type.type = value;
        if(value === 'gps')
        {
            settings.simulation_type.from_duration = $($("#gps_topology [name=from_duration]")[0]).val();
            settings.simulation_type.to_duration = $($("#gps_topology [name=to_duration]")[0]).val();
            
            load_file($("#gps_topology [name=gps_data]")[0], function(e) {
                settings.simulation_type.gps_data = e.target.result;
                execution_handler();
                $("#settings").hide(200);
            }); 
        }
        else if(value === 'static')
        {
            settings.simulation_type.topology = $($("#static_topology [name=topology]")[0]).val();
            settings.simulation_type.number_of_nodes = $($("#static_topology [name=number_of_nodes]")[0]).val();
            settings.simulation_type.node_init_time = $($("#static_topology [name=node_init_time]")[0]).val();
            settings.simulation_type.duration = $($("#static_topology [name=duration]")[0]).val();
            $("#settings").hide(200);
        }
    });
}

function update_style(id)
{
    $("#style_input .style").hide();
    $("#style_input #" + id).show();
    $("#style_input").show(200, function () {
        code_mirror.refresh();
        code_mirror.setValue(settings.style[id]);
    });
    style_mode = id;
}

function handle_error(message)
{
    console.log(message);
    alert(message);
}
function execution_handler()
{
    $.ajax({
        url: "/execute/",
        type: "POST",
        data: JSON.stringify(settings.simulation_type),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data, textStatus, jqXHR) {
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
    if (data === {})
        return;
    $("#speed_slider").attr('min', -100);
    $("#speed_slider").attr('max', 99);
    $("#speed_slider").val(0);
    $("#time_slider").attr('min', data.first_time);
    $("#time_slider").attr('max', data.last_time);
    $("#time_slider").val(data.first_time);
    glob_time = data.first_time;
    current_data = data;
    settings.simulation_data.push({"time": (new Date()).getTime(), "data": data});
    var bounds = [[data.min_lat, data.min_lng], [data.max_lat, data.max_lng]];
    map.map.fitBounds(bounds);
}

function run() {
    $('#run').click();
}

function save_style()
{
    download(settings.style, 'simulation.style', 'json');
}

function load_style()
{
    $("#style_file").click();
}

function save_all()
{
    download(settings, 'simulation.workspace', 'json');
}

function load_all()
{
    $("#settings_file").click();
}

function save_simulation()
{
    if (current_data !== null)
    {
        download(current_data, 'simulation.data', 'json');
    } else
    {
        alert("No simulation to save!");
    }
}

function load_simulation()
{
    $("#simulation_file").click();
}

function show_gps()
{
    $("#gps_log").click();
}

// stolen from https://stackoverflow.com/questions/13405129/javascript-create-and-save-file
function download(data, filename, type) {
    var file = new Blob([JSON.stringify(data)], {type: type});
    if (window.navigator.msSaveOrOpenBlob) // IE10+
        window.navigator.msSaveOrOpenBlob(file, filename);
    else { // Others
        var a = document.createElement("a"),
                url = URL.createObjectURL(file);
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        setTimeout(function () {
            document.body.removeChild(a);
            window.URL.revokeObjectURL(url);
        }, 0);
    }
}