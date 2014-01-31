var exec = require('child_process').exec,
	execSync = require('execSync').run;

var
	terrainSize = 1 << 13,
	leafSize = 1 << 9;

var go = function() {
	for (var y = -terrainSize / 2 ; y < terrainSize / 2 ; y += leafSize) {
		for (var x = -terrainSize / 2 ; x < terrainSize / 2 ; x += leafSize) {
			var code =
				execSync("./fetch-terrain " + x + " " + y + " " + terrainSize + " " + leafSize);
		}
	}
}

go();
