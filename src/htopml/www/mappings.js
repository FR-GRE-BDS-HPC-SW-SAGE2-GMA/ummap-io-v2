function draw_blocks(data)
{
	for (var i in data) {
		//get current
		var mapping = data[i];

		//select div
		div = d3.select("#mapping-"+i);

		//select blocks
		var blocks = div.selectAll("span").data(mapping.status);

		//update
		blocks.transition()
			.attr("class",function (status) {
				if (status.dirty == 1)
					return "dirty";
				else if (status.mapped == 1)
					return "mapped";
				else
					return "unmapped";
			});

		//append blocks
		blocks.enter()
			.append("span")
			.attr("class",function (status) {
				if (status.dirty == 1)
					return "dirty";
				else if (status.mapped == 1)
					return "mapped";
				else
					return "unmapped";
			});

		//exit
		blocks.exit()
			.transition()
			.duration(300)
				.attr("width", 0)
				.remove();
	}
}

function draw_sections(mappings)
{
	var html = "";
	for (var i in mappings)
	{
		//vars
		var infos = "";
		var mapping = mappings[i];
		console.log(mapping);

		//setup uris
		infos += "<table>"
		infos += "<tr><td class='property'>Size: </td><td>" + mapping.size + "</td></tr>";
		infos += "<tr><td class='property'>Segment size: </td><td>" + mapping.segmentSize + "</td></tr>";
		infos += "<tr><td class='property'>Driver: </td><td>" + mapping.driverUri + "</td></tr>";
		infos += "<tr><td class='property'>Local policy: </td><td>" + mapping.localPolicyUri + "</td></tr>";
		infos += "<tr><td class='property'>Global policy: </td><td>" + mapping.globalPolicyUri + "</td></tr>";
		infos += "</table>"

		//appned
		html += "<div class='section'>\
				<h1>Mapping "+ i +"</h1>\
				<div>" + infos + "</div>\
				<br/>\
				<div class='blocks' id='mapping-"+i+"'></div>\
				<div style='clear:both'></div>\
			</div>";
	}

	//apply
	$("#body").html(html);
}

function refresh_loop()
{
	$.getJSON("/ummap-io/mappings.json", function(data){
		//console.log(data);
		draw_blocks(data);
		setTimeout(refresh_loop, 200);
	});
}

function setup_sections()
{
	$.getJSON("/ummap-io/mappings.json", function(data){
		//console.log(data);
		draw_sections(data);
		draw_blocks(data)
		setTimeout(refresh_loop, 200);
	});
}

$(document).ready(function() {
	setup_sections();
});
