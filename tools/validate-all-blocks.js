// gzip-compress-data.js
// Raw compresses data
//

var
	fs = require('fs'),
	path = require('path'),
	execSync = require('execSync').run;

process.nextTick(function() {
	var files = fs.readdirSync('./data');
	console.log('Checking files...');

	for (var i = 0 ; i < files.length ; i ++) {
		var f = files[i];
		if (f === 'meta.json') // don't compress meta
			continue;

		var source = path.join('./data', f);

		console.log('Checking', source, '...');
		var code = execSync("./validate-points " + source);
		console.log('...', (code === 0 ? "VALID" : "NOT-VALID"));
	}
});
