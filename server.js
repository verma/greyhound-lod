var
	http = require('http'),
	fs = require('fs'),
	path = require('path'),
	sys = require('sys'),

	checkTree = require('./check-tree').checkTree;

var
	port = process.env.PORT || 8000,
	dataInfo = require("./data/meta.json");

function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}

var staticMap = {
	"/": [ "run.html", "text/html"],
	"/top": ["top-proj.html", "text/html"],
	"/single": ["view-single.html", "text/html"],
	"/jquery.js": ["jquery.js", "application/javascript; charset=utf-8"],
	"/bluebird.js": ["bluebird.js", "application/javascript; charset=utf-8"],
	"/QuadTree.js": ["QuadTree.js", "application/javascript; charset=utf-8"],
	"/three.js": ["three.js", "application/javascript; charset=utf-8"],
	"/lodash.js": ["lodash.js", "application/javascript; charset=utf-8"],
	"/downloadWorker.js": ["downloadWorker.js", "application/javascript; charset=utf-8"],
	"/FirstPersonControls.js": ["FirstPersonControls.js", "application/javascript; charset=utf-8"],
	"/jquery.nouislider.min.js": ["jquery.nouislider.min.js", "application/javascript; charset=utf-8"],
	"/jquery.nouislider.min.css": ["jquery.nouislider.min.css", "text/css; charset=utf-8"]
};

var fourOhFour = function(res) {
	res.writeHead(404, {
		'Content-Type': 'text/plain' });
	res.write('File not found');
	return res.end();
}

var totalBytesTransferred = 0;

var handlePointRequest = function(url, req, res) {
	var rx = /\/points\/(.*)/g;
	var arr = rx.exec(url);

	var json = function(obj) {
		res.writeHead(200, {
			'Content-Type': 'application/json; charset=utf-8',
		});
		res.write(JSON.stringify(obj))
		res.end();
	}

	var parts = arr[1].split(/\//);
	if (parts[0] === 'bounds') {
		return json({
			x: -dataInfo.terrainSize/2, z: -dataInfo.terrainSize/2,
			width: dataInfo.terrainSize, height: dataInfo.terrainSize,
			leaf: { width: dataInfo.leafSize, height: dataInfo.leafSize }});
	}
	if (parts[0] === 'totalTransferred') {
		return json({
			totalTransferred: totalBytesTransferred
		});
	}
	else if (parts[0] === 'cloud') {
		var x = parts[1];
		var z = parts[2];
		var w = parts[3];
		var h = parts[4];

		if (x && z && w && h) {
			x = parseInt(x);
			z = parseInt(z);
			w = parseInt(w);
			h = parseInt(h);

			var fn = "data." + x + "." + z + "." +
				(x+w) + "." + (z+h);
			var p = path.join("./data", fn);

			return fs.stat(p, function(err, stats) {
				if(err) return fourOhFour(res);

				var needGzip = process.env.NOGZIP ? false : true;
				console.log(">>>", p, (needGzip ? "(gzip)" : ""));

				if (needGzip) {
					res.writeHead(200, {
						'Content-Type': 'application/octet-stream',
						'Content-Encoding': 'gzip'
					});
				}
				else {
					res.writeHead(200, {
						'Content-Type': 'application/octet-stream'
					});
				}

				totalBytesTransferred += stats.size;
				fs.createReadStream(p).pipe(res);
			});
		}
	}

	return fourOhFour(res);
}


var fileMap = function(url, res) {
	var r = staticMap[url];
	if (!r)
		return fourOhFour(res);

	res.writeHead(200, {
		'Content-Type': r[1]
	});
	fs.createReadStream(r[0]).pipe(res);
}

var go = function() {
	console.log('Data source specs:');
	console.log('    terrrain size:', dataInfo.terrainSize);
	console.log('        leaf size:', dataInfo.leafSize);

	checkTree('./data', dataInfo.terrainSize, dataInfo.leafSize);
	console.log('Data tree looks good.');

	var server = http.createServer(function(req, res) {
		if (req.url.indexOf('/points') != -1) {
			return handlePointRequest(req.url, req, res);
		}

		return fileMap(req.url, res);
	});

	server.listen(port, function() {
		console.log('Listening on port:', port);
	});
};

process.nextTick(go);
