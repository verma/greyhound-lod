var exec = require('child_process').exec,
	execSync = require('execSync').run,
	fs = require('fs'),
	pg = require('pg'); 

var
	terrainSize = 1 << 15,
	leafSize = 1 << 8;

var getDataDimensions = function(cb) {
	var conString = "postgres://lidar@localhost/lidar";

	function parseBox(box) {
		var parts =
			box.substring(4, box.length - 1).replace(',', ' ').split(' ');
		return {
			left: parseFloat(parts[0]),
			top: parseFloat(parts[1]),
			right: parseFloat(parts[2]),
			bottom: parseFloat(parts[3])
		};
	}

	var client = new pg.Client(conString);
	client.connect(function(err) {
		if (err) return cb(err);
		client.query("select st_extent(pa::geometry) as e from lidar", function(err, res) {
			if (err) return cb(err);
			var box = parseBox(res.rows[0].e);
			client.end();

			cb(null, box);
		});
	});
};

var getRegion = function(cb) {
	// get a region of interest
	getDataDimensions(function(err, dims) {
		if (err) return cb(err);

		var dr = Math.min(dims.right - dims.left, dims.bottom - dims.top);
		var center = {
			x: dims.left + (dims.right - dims.left) / 2,
			y: dims.top + (dims.bottom - dims.top) / 2 };

		var s = 1;

		cb(null, {
			left: center.x - dr / 2.0 * s,
			right: center.x + dr / 2.0 * s,
			top: center.y - dr / 2.0 * s,
			bottom: center.y + dr / 2.0 * s,

			size: dr * s
		});
	});
};

var fetch = function(box, x, y) {
		var f = 1.0 / terrainSize;
		var fx = function(v) { return box.left + (box.right - box.left) * v; };
		var fy = function(v) { return box.top + (box.bottom - box.top) * v; };

		var tscale = terrainSize / box.size;
		var normx = box.left, normy = box.top;

		var lf = (terrainSize - x - leafSize) * f, rf = (terrainSize - x) * f,
		tf = y * f, bf = (y+leafSize) * f;

		var l = fx(lf), r = fx(rf), t = fy(tf), b = fy(bf);
		var command = "./fetch-terrain " + (x - terrainSize/2) + " " + (y - terrainSize/2) + " " + terrainSize + " " + leafSize + " " +
			l + " " + r + " " + t + " " + b + " " + tscale + " " + normx + " " + normy;
		console.log(command);
		var code =
			execSync(command);

		var filename = "data/data." + (x - terrainSize/2) + "." + (y - terrainSize/2) + "."
		+ (x-terrainSize/2+leafSize) + "." + (y-terrainSize/2+leafSize);

		var code =
			execSync("./validate-points " + filename);
		if (code !== 0)
			throw new Error("Validation failed");
}

var generateAll = function(xstart, xend, ystart, yend) {
	getRegion(function(err, box) {
		console.log("generate new", box);

		for (var y = ystart ; y < yend ; y += leafSize) {
			for (var x = xstart ; x < xend ; x += leafSize) {
				fetch(box, x, y);
			}
		}
	});
}

var rebuildMissing = function(xstart, xend, ystart, yend) {
	getRegion(function(err, box) {
		console.log("rebuild", box);

		for (var y = ystart ; y < yend ; y += leafSize) {
			for (var x = xstart ; x < xend ; x += leafSize) {
				var filename = "data/data." + (x - terrainSize/2) + "." + (y - terrainSize/2) + "."
					+ (x-terrainSize/2+leafSize) + "." + (y-terrainSize/2+leafSize);

				if (!fs.existsSync(filename)) {
					console.log('rebuilding:', filename);
					fetch(box, x, y);
				}
			}
		}
	});
}

process.nextTick(function() {
	var xstart = 0, xend = terrainSize;
	var ystart = 0, yend = terrainSize;

	var args = process.argv.slice(2);
	console.log(args);

	var rebuild = (args[0] === "rebuild")
	if (rebuild)
		args = args.slice(1);

	var f = rebuild ? rebuildMissing : generateAll;
	if (args.length > 0) {
		var xflag = parseInt(args[0]);
		var yflag = parseInt(args[1]);

		if (isNaN(xflag) || xflag > 1 ||
			isNaN(yflag) || yflag > 1)
			throw new Error("Flags should be between 1 and 0")

		xstart = xflag * terrainSize / 2;
		xend = xstart + terrainSize / 2;

		ystart = yflag * terrainSize / 2;
		yend = ystart + terrainSize / 2;
	}

	console.log("(", xstart, ",", ystart, ") -> (", xend, ",", yend, ")");

	f(xstart, xend, ystart, yend);
});
