// points-lod.js
// quadtree lod testing
//

(function() {
	"use strict";

	var start = function() {
		console.log('Starting up...');

		var canvas = document.getElementById("canvas");
		var context = canvas.getContext("2d");

		var eyePos = null;

		var width = $(canvas).width();
		var height = $(canvas).height();
		var MAX_DEPTH = 5;

		var qTree = new QuadTree(-width/2, -height/2, width, height, MAX_DEPTH);

		var refreshDisplay = function() {
			requestAnimationFrame(refreshDisplay);

			context.clearRect(0, 0, width, height);

			if (eyePos === null)
				return; // no valid eye position yet

			// draw all nodes
			//
			var queryPos = {
				x: eyePos.x - width / 2,
				y: eyePos.y - height / 2
			};

			qTree.lodQuery(queryPos, function(n) {
				var r = 100 + Math.floor(((MAX_DEPTH - n.depth) / MAX_DEPTH) * 155);

				context.fillStyle = "hsl(" + Math.floor(360 * n.depth/MAX_DEPTH) + ", 50%, 50%)";
				context.fillRect(
					n.x + width/2,
					n.y + height/2,
					n.w, n.h);
			});

			// draw where our eye goes
			context.beginPath();
			context.arc(eyePos.x, eyePos.y, 6, 0, Math.PI*2, true);
			context.closePath();
			context.fillStyle = "#fff";
			context.fill();

			var spheres = qTree.LODSpheres();
			for (var i = 0 ; i < spheres.length ; i++) {
				context.beginPath();
				context.arc(eyePos.x, eyePos.y, spheres[i], 0, Math.PI*2, true);
				context.closePath();
				context.strokeStyle = "hsl(" + Math.floor(360 * i / spheres.length) + ", 100%, 50%)";
				context.stroke();
			}
		};

		// attach click handlers
		//
		$("#canvas").on("mousemove", function(e) {
			e.preventDefault();

			var parentOffset = $(this).parent().offset();
			eyePos = {
				x: e.pageX - parentOffset.left,
				y: e.pageY - parentOffset.top,
			};

			console.log(eyePos);
		});

		$("#canvas").on("mouseleave", function(e) {
			e.preventDefault();
			eyePos = null;
		});

		refreshDisplay();
	}

	$(start);
})();
