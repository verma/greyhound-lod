// gzip-compress-data.js
// Raw compresses data
//

var
	fs = require('fs'),
	path = require('path'),
	zlib = require('zlib');

var tasker = function(max, cb) {
	var currentIndex = 0;
	var dispath = null;

	dispatch = function() {
		cb(currentIndex, function() {
			currentIndex++;
			if (currentIndex < max)
				dispatch();
		});
	}

	dispatch();
}

process.nextTick(function() {
	var files = fs.readdirSync('./data');
	console.log('Compressing', files.length, 'files...');

	tasker(files.length, function(i, done) {
		var f = files[i];
		if (f === 'meta.json') // don't compress meta
			return process.nextTick(done);

		var source = path.join('./data', f);
		var dest = path.join('./data', f + '.gzip');

		var s = fs.createReadStream(source);
		var w = fs.createWriteStream(dest);

		s.on('end', function() {
			fs.unlinkSync(source);
			fs.renameSync(dest, source);

			done();
		});

		console.log('...', f, ':::', i+1, '/', files.length);
		s.pipe(zlib.createGzip()).pipe(w);
	});
});
