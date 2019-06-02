const mappa = new Mappa('Mapbox', 'pk.eyJ1IjoicGV0ZXJnam9lbCIsImEiOiJjajNhY2hjb2owMDVhMzJud2o0bTJ5YjlvIn0.cYd9pq3oAEDLL9trzPBvRA');
let map_canvas;
let map;
let glob_time = 0;
let time_slider_active = false;
let current_data = null;
let pause = true;

let rainbow = new Rainbow();
rainbow.setSpectrum('red', 'yellow', 'green');
rainbow.setNumberRange(-110, 0);

let protocol_replay_data;
let node_text_state;
let init_node_text_state;

let settings = {
        "style": {
            "node_style":
                "node.mode = 0; // 1 = display nodes, 2 = with replay features\n\n" +
                "if(node.mode === 1) {\n" +
                "   canvas.strokeWeight(1);\n" +
                "   canvas.stroke(0, 0, 0);\n" +
                "   canvas.fill('rgba(255,255,255, 1)');\n\n" +
                "} else if (node.mode === 2) {\n" +
                "    canvas.strokeWeight(1);\n" +
                "    canvas.stroke(0, 0, 0);\n\n" +
                "    let color = {'i': 'rgba(255,255,255,1)', 'w': 'rgba(178,34,34,1)', 'd': 'rgba(30,144,255,1)'}; // LMAC colors\n" +
                "    let f;\n\n" +
                "    replay_update_node_text_state(node.id);\n" +
                "    if (color.hasOwnProperty(node_text_state[node.id]))\n" +
                "        f = color[node_text_state[node.id]];\n" +
                "    else\n" +
                "        f = 'rgba(127,255,0,1)';\n" +
                "    canvas.fill(f);\n\n" +
                "} else { \n" +
                "    canvas.noFill(); canvas.noStroke();\n" +
                "}\n",
            "node_extrapolation": "",
            "edge_style":
                "/* usefull functions: \n" +
                " * ids_in_link(from, to, id1, id2)  - return true of edges has nodes with given ids. id2 is optional\n" +
                " * map.map.distance(from, to)      - returns distance of edge\n" +
                " * rainbow.colourAt(edge.rssi)      - returns a colour based on the rssi of edge\n" +
                " */\n\n" +
                "edge.status = 0; /* Set to 1 to show edges */\n\n" +
                "if(edge.status > 0) {\n" +
                "    canvas.strokeWeight(3);\n\n" +
                "    canvas.stroke('#' + rainbow.colourAt(edge.rssi));\n" +
                "} else {\n" +
                "    canvas.noFill(); canvas.noStroke();\n" +
                "}\n",
            "edge_extrapolation": "",
        },
        "simulation_type": {},
        "simulation_data": []
    }
;
let style_mode = "";
let code_mirror;

const options = {
    lat: 0,
    lng: 0,
    zoom: 4,
    keyboard: false,
    style: 'mapbox.light'
};

$(document).ready(function () {
    // listen for changes
    $.getJSON("/models/", function (data) {
        if (data === null || data.hasOwnProperty("error") || !data.hasOwnProperty("models")) {
            if (data === null || !data.hasOwnProperty("models")) {
                handle_error("No/Wrong data received!");
                console.log(data);
            } else
                handle_error(data.error);
        } else {
            add_models(data.models);
        }
    });

    // initialize the map
    let show_map = function (p) {
        p.setup = function () {
            map_canvas = p.createCanvas(p.windowWidth, p.windowHeight - 30);
            map = mappa.tileMap(options);
            map.overlay(map_canvas);
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
            if (current_data !== null) {
                p.clear();
                let speed = $("#speed_slider").val();
                if (speed < 0)
                    speed = 1 / Math.abs(speed);
                else
                    speed = 1 + Math.abs(speed);
                if (!time_slider_active && !pause) {
                    glob_time = glob_time + ((1000 / p.frameRate()) * speed);
                    $("#time_slider").val(glob_time);
                } else if (time_slider_active) {
                    glob_time = parseFloat($("#time_slider").val());
                } else if (p.keyIsDown(p.LEFT_ARROW)) {
                    glob_time -= (speed / p.frameRate()) * 10;
                } else if (p.keyIsDown(p.RIGHT_ARROW)) {
                    glob_time += (speed / p.frameRate()) * 10;
                }
                $("#time_val").html(glob_time);
                $("#speed_val").html("x " + speed);

                // draw edges in the bottom
                for (let id in current_data.nodes) {
                    const data = node_data(id, glob_time);
                    if (data === null)
                        continue;

                    const dot = map.latLngToPixel(data);
                    let edges = node_edges(id, glob_time);
                    for (let eid in edges) {
                        let edge = edges[eid];
                        const odata = node_data(edge.dest, glob_time);
                        const odot = map.latLngToPixel(odata);

                        p.noFill();
                        p.noStroke();
                        if (edge.hasOwnProperty("status") && edge.status == 0)
                            continue;

                        data.id = parseInt(id);
                        odata.id = parseInt(edge.dest);
                        _show_edge(p, edge, data, odata, dot, odot);

                        let length = Math.sqrt(Math.pow(dot.x - odot.x, 2) + Math.pow(dot.y - odot.y, 2));
                        let dy = ((dot.y - odot.y) / length);
                        let dx = ((dot.x - odot.x) / length);
                        let rad = Math.abs(dx) < Math.abs(dy) ? Math.asin(dy) : Math.acos(dx);
                        let cs = Math.cos(rad + Math.PI / 2);
                        let ss = Math.sin(rad + Math.PI / 2);
                        //p.curve(dot.x-10, dot.y-10, dot.x, dot.y, odot.x, odot.y ,odot.x+10, odot.y+10);
                        let midx = (odot.x + dot.x) / 2 - cs * Math.min(10, length / 2);
                        let midy = (odot.y + dot.y) / 2 - ss * Math.min(10, length / 2);


                        p.line(dot.x, dot.y, midx, midy);
                        p.line(midx, midy, odot.x, odot.y);
                        ss *= 5;
                        cs *= 5;
                        dx *= 5;
                        dy *= 5;
                        p.triangle(midx - dx, midy - dy, midx + cs, midy + ss, midx - cs, midy - ss);
                        // add some top here!
                    }
                }

                for (let id in current_data.nodes) {
                    const data = node_data(id, glob_time);

                    if (data === null)
                        continue;

                    const dot = map.latLngToPixel(data);
                    data.id = id;
                    p.noFill();
                    p.noStroke();

                    _show_node(p, data, dot);

                    p.ellipse(dot.x, dot.y, diam, diam);
                    p.fill(0, 0, 0);
                    p.stroke(0, 0, 0);
                    p.strokeWeight(0.8);
                    p.text(node_text_state[id], dot.x - (diam / 3) + 3, dot.y + (diam / 4));
                }

                if (glob_time >= current_data.last_time) {
                    $("#stop").click();
                    $("#play").click();
                    node_text_state = init_node_text_state;
                }
            }
        };
        /*p.mouseClicked = function ()
         {
         if (current_data !== null)
         {
         for (let id in current_data.nodes)
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
            let speed = $("#speed_slider").val();
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
        p.windowResized = function () {
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
        if (current_data !== null) {
            pause = false;
            $("#play").attr('class', 'active');
            $("#pause").attr('class', '');
            $("#stop").attr('class', '');
        }
    });
    $("#pause").click(function () {
        if (current_data !== null) {
            pause = true;
            $("#play").attr('class', '');
            $("#pause").attr('class', 'active');
            $("#stop").attr('class', '');
        }
    });
    $("#stop").click(function () {
        pause = true;
        if (current_data) {
            $("#time_slider").val(current_data.first_time);
            glob_time = current_data.first_time;
        }
        $("#play").attr('class', '');
        $("#pause").attr('class', '');
        $("#stop").attr('class', 'active');
    });
    $(".top_menu").hover(function () {
        $("ul", this).slideDown(100);
        let width = $(this).width();
        $('li', this).each(function () {
            width = Math.max($(this).width(), width);
        });
        $('li', this).each(function () {
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
            let lines = e.target.result;
            let data = JSON.parse(lines);
            settings.style = data;
            set_styles();
        });
    });
    $("#settings_file").change(function () {
        load_file(this, function (e) {
            let lines = e.target.result;
            let data = JSON.parse(lines);
            settings = data;
            set_styles();
        });
    });
    $("#simulation_file").change(function () {
        load_file(this, function (e) {
            let lines = e.target.result;
            let data = JSON.parse(lines);
            show_simulation(data);
        });
    });
    $("#gps_log").change(function () {
        load_file(this, function (e) {
            // this could be done in pure javascript, no need to call backend, really.
            settings.simulation_type = {};
            settings.simulation_type.gps_data = e.target.result;
            settings.simulation_type.type = "log";
            execution_handler();
        });
    });
    $("#gps_log_rssi").change(function () {
        load_file(this, function (e) {
            // this could be done in pure javascript, no need to call backend, really.
            settings.simulation_type = {};
            settings.simulation_type.gps_data = e.target.result;
            settings.simulation_type.type = "log+rssi";
            execution_handler();
        });
    });
    $("#load_protocol").change(function () {
        load_file(this, function (e) {
            protocol_replay_data = {};
            e.target.result.split('\n').forEach(line => {
                let split = line.split(',');
                if (split[0] === "")
                    return;

                if (!protocol_replay_data.hasOwnProperty(split[1]))
                    protocol_replay_data[split[1]] = {};

                protocol_replay_data[split[1]][split[0]] = split[2];
            });
        });
    });
    $("#load_communication").change(function () {
        load_file(this, function (e) {
            let edges = {};
            e.target.result.split('\n').forEach(line => {
                let split = line.split(',');
                if (split[0] === 'drop')
                    return;

                let tx_id = parseInt(split[1]);
                let rx_id = parseInt(split[2]);
                let tx_start = split[8];
                let rx_start = split[9];

            });

        });
    });
});

function load_file(element, handler) {
    if (!element.files[0]) {
        alert("Could not load the file");
    } else {
        let file = element.files[0];
        let fr = new FileReader();
        fr.onload = handler;
        fr.readAsText(file);
    }
}

function set_styles() {
    set_style('node_style');
    set_style('node_extrapolation');
    set_style('edge_extrapolation');
    set_style('edge_style');
}

function set_style(style_mode) {
    if (style_mode === 'node_style') {
        eval("_show_node = function(canvas, node, point){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'node_extrapolation') {
        eval("_extrapolate_node = function(pre, post, delta){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'edge_style') {
        eval("_show_edge = function(canvas, edge, from, to, from_point, to_point){" + settings.style[style_mode] + "}");
    } else if (style_mode === 'edge_extrapolation') {
        eval("_extrapolate_edge = function(pre, post, delta){" + settings.style[style_mode] + "}");
    } else {
        alert("Uknown style setting " + style_mode);
    }
}


let _show_node = function (canvas, node, point) {
};
let _show_edge = function (canvas, edge, from, to, from_point, to_point) {
};
let _extrapolate_node = function (pre, post, delta) {
};
let _extrapolate_edge = function (pre, post, delta) {
};

function node_data(id, time) {
    if (current_data.nodes[id][0].timestamp > time ||
        current_data.nodes[id][current_data.nodes[id].length - 1].timestamp < time)
        return null;

    let post = 0;
    while (current_data.nodes[id][post].timestamp < time)
        ++post;

    if (post === 0)
        return current_data.nodes[id][post];

    let part = current_data.nodes[id][post].timestamp - current_data.nodes[id][post - 1].timestamp;
    part = (time - current_data.nodes[id][post - 1].timestamp) / part;
    let pre = $.extend(true, {}, current_data.nodes[id][post - 1]);
    let postd = current_data.nodes[id][post];
    pre.lat += (postd.lat - pre.lat) * part;
    pre.lng += (postd.lng - pre.lng) * part;
    _extrapolate_node(pre, postd, part);

    return pre;
}

function node_edges(id, time) {
    let res = [];
    if (current_data.edges[id] == null) {
        return res;
    }

    for (let eid in current_data.edges[id]) {
        if (current_data.edges[id][eid][0].timestamp > time ||
            current_data.edges[id][eid][current_data.edges[id][eid].length - 1].timestamp < time)
            continue;
        let post = 0;
        while (current_data.edges[id][eid][post].timestamp < time)
            ++post;
        if (post === 0)
            res.push(current_data.edges[id][eid][post]);
        else {
            let part = current_data.edges[id][eid][post].timestamp - current_data.edges[id][eid][post - 1].timestamp;
            part = (time - current_data.edges[id][eid][post - 1].timestamp) / part;
            let pre = $.extend(true, {}, current_data.edges[id][eid][post - 1]);
            _extrapolate_edge(pre, current_data.nodes[id][post], part);
            res.push(pre);
        }
    }
    return res;
}

function update_settings(value) {
    $("#type").val(value);
    $(".settings").hide();
    if (value === 'static') {
        $("#static_topology.settings").show();
    } else if (value === 'gps') {
        $("#gps_topology.settings").show();
    } else {
        handle_error("Unknown Simulation-type selected: " + value);
        return;
    }
    $("#settings").show(200);
    $("#settings form").prop("onclick", null).off("click");
    $("#settings form").submit(function (e) {
        e.preventDefault();
        settings.simulation_type = {};
        settings.simulation_type.model = $("#model").val();
        settings.simulation_type.type = value;
        if (value === 'gps') {
            settings.simulation_type.from_duration = $($("#gps_topology [name=from_duration]")[0]).val();
            settings.simulation_type.to_duration = $($("#gps_topology [name=to_duration]")[0]).val();

            load_file($("#gps_topology [name=gps_data]")[0], function (e) {
                settings.simulation_type.gps_data = e.target.result;
                execution_handler();
                $("#settings").hide(200);
            });
        } else if (value === 'static') {
            settings.simulation_type.topology = $($("#static_topology [name=topology]")[0]).val();
            settings.simulation_type.number_of_nodes = $($("#static_topology [name=number_of_nodes]")[0]).val();
            settings.simulation_type.node_init_time = $($("#static_topology [name=node_init_time]")[0]).val();
            settings.simulation_type.duration = $($("#static_topology [name=duration]")[0]).val();
            execution_handler();
            $("#settings").hide(200);
        }
    });
}

function update_style(id) {
    $("#style_input .style").hide();
    $("#style_input #" + id).show();
    $("#style_input").show(200, function () {
        code_mirror.refresh();
        code_mirror.setValue(settings.style[id]);
    });
    style_mode = id;
}

function handle_error(message) {
    console.log(message);
    alert(message);
}

function execution_handler() {
    $("#wait").show(200);
    $("#pause").click();
    $.ajax({
        url: "/execute/",
        type: "POST",
        data: JSON.stringify(settings.simulation_type),
        contentType: 'application/json; charset=utf-8',
        dataType: 'json',
        success: function (data, textStatus, jqXHR) {
            $("#wait").hide(200);
            if (data === null) {
                handle_error("An unknown response from the server");
            } else if (data.hasOwnProperty("error")) {
                handle_error(data.error);
            } else {
                show_simulation(data);
            }
        },
        error: function (jqXHR, status, errorThrown) {
            $("#wait").hide(200);
            handle_error(status);
            console.log(status);
            console.log(errorThrown);
            console.log(jqXHR);
        }
    });
}

function handle_load() {
    alert("Loading is not yet implemented");
}

function add_models(models) {
    let data = "";
    for (let i in models) {
        let first = (data.length === 0);
        data += "<option value='" + models[i] + "' ";
        if (first)
            data += "selected=selected "
        data += ">" + models[i] + "</option>";
    }
    $("#model").html(data);
}


// lets do the actual display!
function show_simulation(data) {
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

    node_text_state = {};
    for (let nid in current_data.nodes) {
        node_text_state[nid] = nid;
    }

    init_node_text_state = node_text_state;

    for (let i = 0; i < current_data.nodes.length; i++) {
        console.log(current_data.nodes[i]);
    }
    settings.simulation_data.push({"time": (new Date()).getTime(), "data": data});
    let bounds = [[data.min_lat, data.min_lng], [data.max_lat, data.max_lng]];
    map.map.fitBounds(bounds);
}

function run() {
    $('#run').click();
}

function save_style() {
    download(settings.style, 'simulation.style', 'json');
}

function load_style() {
    $("#style_file").click();
}

function save_all() {
    download(settings, 'simulation.workspace', 'json');
}

function load_all() {
    $("#settings_file").click();
}

function save_simulation() {
    if (current_data !== null) {
        download(current_data, 'simulation.data', 'json');
    } else {
        alert("No simulation to save!");
    }
}

function load_simulation() {
    $("#simulation_file").click();
}

function show_gps() {
    $("#gps_log").click();
}

function show_gps_rssi() {
    $("#gps_log_rssi").click();
}

function load_protocol_log() {
    if (current_data == null) {
        alert('No gps log has been loaded');
        return;
    }

    $("#load_protocol").click();
}

function load_communication_log() {
    $("#load_communication").click();
}

function ids_in_link(node1, node2, id1, id2) {
    if (typeof id2 !== 'undefined') {
        return (id1 === node1.id || id1 === node2.id) && (id2 === node1.id || id2 === node2.id);
    }

    return id1 === node1.id || id1 === node2.id;
}

function replay_update_node_text_state(nid) {
    let keys = Object.keys(protocol_replay_data[nid]).map(val => parseFloat(val));
    let kl = keys.length;
    if (kl === 1 && keys[0] < glob_time) {
        node_text_state[nid] = protocol_replay_data[nid][keys[0]];
        return;
    }

    for (let i = 0; i < kl; i++) {
        if (i + 1 === kl && keys[i] < glob_time) {
            node_text_state[nid] = protocol_replay_data[nid][keys[i]];
            return;
        } else if (keys[i] < glob_time && keys[i + 1] > glob_time) {
            node_text_state[nid] = protocol_replay_data[nid][keys[i]];
            return;
        }
    }
}

// stolen from https://stackoverflow.com/questions/13405129/javascript-create-and-save-file
function download(data, filename, type) {
    let file = new Blob([JSON.stringify(data)], {type: type});
    if (window.navigator.msSaveOrOpenBlob) // IE10+
        window.navigator.msSaveOrOpenBlob(file, filename);
    else { // Others
        let a = document.createElement("a"),
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
