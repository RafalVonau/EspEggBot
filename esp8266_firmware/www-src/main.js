var gcode_data = new Array()
var gcode_idx = 0;
var gx = 0;
var gy = 0;
var gp = 1;
var scale = 0.5;

var canvas = null;
var ctx = null;

function addMessage(m) {
	console.log(m);
}

function drawProgress()
{
	var elem = document.getElementById("myBar");
	var width = (gcode_idx * 100)/gcode_data.length;
	elem.style.width = width + "%";
} 

function clearCanvas()
{
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	scale = (canvas.width*1.0)/3200.0;
}

function startDraw()
{
	gcode_idx = 4;
	gx = canvas.width/2;
	gy = canvas.height/2;
	gp = 1;
}

function drawLine(x,y,g)
{
	x = gx + x;
	y = gy - y;
	if (gp == 0) {
		// set line stroke and line width
		if (g == 1) ctx.strokeStyle = 'gray'; else ctx.strokeStyle = 'red';
		ctx.lineWidth = 2;
		// draw a red line
		ctx.beginPath();
		ctx.moveTo(gx, gy);
		ctx.lineTo(x, y);
		ctx.stroke();
	}
	gx = x;
	gy = y;
	/* Roll over */
	while (gx >= canvas.width)   gx -= canvas.width;
	while (gy >= canvas.height)  gy -= canvas.height;
	while (gx <= -canvas.width)  gx += canvas.width;
	while (gy <= -canvas.height) gy += canvas.height;
}


function parseCommand(cmd, g)
{
	var res = cmd.split(",");
	if (res[0] == "SM") {
		drawLine(res[3]*1.0*scale ,res[2]*1.0*scale, g );
	} else if (res[0] == "SP") {
		gp = res[1]*1;
	}
}


function sendCommand()
{
	if (gcode_idx < gcode_data.length) {
		c = gcode_data[gcode_idx++];
		//parseCommand(c, 0);
		//drawProgress();
		$.ajax({
			url:"post",
			type:"POST",
			data:{cmd: c},
			dataType:"html",
			success:function(data) {
				console.log('Cmd: ' + data);
				parseCommand(data, 0);
				drawProgress();
			}
		});
	}
}

function gotGCODEData(data)
{
	gcode_data = data.trim().split(/\s*[\r\n]+\s*/g);
	clearCanvas();
	startDraw();
	
	for (i = 4; i < gcode_data.length; ++i) {
		c = gcode_data[i];
		parseCommand(c, 1);
	}
	startDraw();
	/* console.log(gcode_data); */
	sendCommand();
}

function startEvents() 
{
	var es = new EventSource('/events');
	es.onopen = function(e) {
		addMessage("Events Opened");
	};
	es.onerror = function(e) {
		if (e.target.readyState != EventSource.OPEN) {
			addMessage("Events Closed");
		}
	};
	es.onmessage = function(e) {
		addMessage("Event: " + e.data);
	};
	es.addEventListener('cmd', function(e) {
		addMessage("Event[cmd]: " + e.data);
		if (e.data == "OK") {
			sendCommand();
		}
	}, false);
}

$(document).ready(function(){
	canvas = document.querySelector('#canvas');
	if (!canvas.getContext) return;
	ctx = canvas.getContext('2d');
	startEvents();
	$("#but_upload").click(function(){
		var fd = new FormData();
		var files = $('#file')[0].files;
		if (files.length > 0 ){
			fd.append('filename',files[0]);
			$.ajax({
				type: "POST",
				url: "https://tymek.duckdns.org/eggbot/togcode.py",
				data: fd,
				contentType: false,
				processData: false,
				success: function(response){
					if (response != 0) {
						gotGCODEData(response);
					} else {
						alert('file not uploaded');
					}
				},
			});
			$('div#infoModal').addClass('is-active');
		} else {
			alert("Please select a file.");
		}
	});
});