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

		var qTree = new QuadTree(0, 0, width, height, 10);

		var refreshDisplay = function() {
			requestAnimationFrame(refreshDisplay);

			context.clearRect(0, 0, width, height);

			if (eyePos === null)
				return; // no valid eye position yet

			// draw all nodes
			//
			eyePos = { x: 769, y: 387 };
			qTree.lodQuery(eyePos, function(n) {
				context.rect(n.x, n.y, n.w, n.h);
			});

			context.strokeStyle = "#999";
			context.stroke();

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