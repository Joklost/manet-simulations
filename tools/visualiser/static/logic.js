

$(document).ready(function() {
    Plotly.d3.csv('https://raw.githubusercontent.com/plotly/datasets/c34aaa0b1b3cddad335173cb7bc0181897201ee6/2011_february_aa_flight_paths.csv', function(err, rows){
        function unpack(rows, key) {
            return rows.map(function(row) { return row[key]; });}

        function getMaxOfArray(numArray) {
            return Math.max.apply(null, numArray);
        }

        var traces = [];
        var count = unpack(rows, 'cnt');
        var startLongitude = unpack(rows, 'start_lon');
        var endLongitude = unpack(rows, 'end_lon');
        var startLat = unpack(rows, 'start_lat');
        var endLat = unpack(rows, 'end_lat');
        for(var j = 0; j < 10; ++j)
        {
            traces.push([]);
            for ( var i = 0 ; i < count.length; i++ ) {
                var opacityValue = count[i]/getMaxOfArray(count);

                var result = {
                    name: "edge" + i,
                    type: 'scattergeo',
                    locationmode: 'USA-states',
                    lon: [ startLongitude[i] , endLongitude[i] ],
                    lat: [ startLat[i] , endLat[i] ],
                    mode: 'lines',
                    line: {
                        width: j,
                        color: 'red'
                    },
                    opacity: opacityValue
                };

                traces[j].push(result);
            }
        }

      var frames = [];
      var steps = 10;
      for (i = 0; i < steps; i++) {
        frames.push({
          name: i,
          data: traces[i]
        });
      }

      var sliderSteps = [];
      for (i = 0; i < steps; i++) {
        sliderSteps.push({
          method: 'animate',
          label: "",
          args: [[i], {
            mode: 'immediate',
            transition: {duration: 300},
            frame: {duration: 300, redraw: false},
          }]
        });
      }


        var layout = {
            title: 'Feb. 2011 American Airline flight paths',
            showlegend: false,
            geo:{
                scope: 'north america',
                projection: {
                    type: 'azimuthal equal area'
                },
                showland: true,
                landcolor: 'rgb(243,243,243)',
                countrycolor: 'rgb(204,204,204)'
            },
            updatemenus: [{
                  x: 0,
                  y: 0,
                  yanchor: 'top',
                  xanchor: 'left',
                  showactive: true,
                  direction: 'left',
                  type: 'buttons',
                  pad: {t: 87, r: 10},
                  buttons: [{
                    method: 'animate',
                    args: [null, {
                      mode: 'immediate',
                      fromcurrent: true,
                      transition: {duration: 300},
                      frame: {duration: 500, redraw: false}
                    }],
                    label: 'Play'
                  }, {
                    method: 'animate',
                    args: [[null], {
                      mode: 'immediate',
                      transition: {duration: 0},
                      frame: {duration: 0, redraw: false}
                    }],
                    label: 'Pause'
                  }]
                }],
                sliders: [{
                  pad: {l: 130, t: 55},
                  currentvalue: {
                    visible: true,
                    prefix: 'Millisecond:',
                    xanchor: 'right',
                    font: {size: 20, color: '#666'}
                  },
                  steps: sliderSteps
                }]
        };

        Plotly.plot('map', {data:traces, layout:layout, frames: frames, showLink: false});

    });

    // listen for changes
    $("#type").change(function(el) {
        update_settings(this);
    });
    update_settings($("#type"));
});

function update_settings(el)
{
    // we could just show/hide these guys instead of add/remove, but to lazy to do it now :)
    console.log(el);
    var value = $(el).val();
    var handler = null;
    if(value == 'static')
    {
        $("#settings #content").html("<p>Topology: <select name=topology id=topology><option selected=selected value=grid >Grid</option></select></p>" +
                            "<p>Number of nodes: <input type='text' name='number_of_nodes' value='64' /></p>" +
                            "<p>Initialization duration of nodes: <input type='text' name='node_init_time' value='10000' /></p>" +
                            "<p>Duration: <input type='text' name='duration' value='100000' /></p>" +
                            "<input type=submit value='Run'/>");
        handler = handle_static;
    }
    else if(value == 'gps')
    {
        $("#settings #content").html("<p class='error'>Not yet implemented</p>");
        handler = handle_gps;
    }
    else if(value == 'load')
    {
        $("#settings #content").html("<p class='error'>Not yet implemented</p>");
        handler = handle_load;
    }
    else
    {
        $("#settings #content").html("<p class='error'>Unknown Simulation-type selected: " + val + "</p>");
        handler = handle_error;
    }
    $("#settings").submit(function(e) {
        e.preventDefault();
        handler();
    });

}

function handle_static()
{
    var type = $("#settings #topology").val();
    if(type == 'grid')
    {
        console.log($("#settings").serialize());
        $.post('/execute/', $("#settings").serialize(),
            function(data)
            {
                alert("Got data!");
            });
    }
    else
    {
        alert("Unknown topology type selected");
    }
    alert("static");
}

function handle_load()
{
    alert("Loading is not yet implemented");
}

function handle_gps()
{
    alert("GPS loading is not yet implemented");
}

function handle_error()
{
    alert("An error occurred, could not handle unknown simulation-type");
}