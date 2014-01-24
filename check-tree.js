// check-tree.js
// called by server when it first starts, making sure the
// entire data tree is present
//

var
	fs = require('fs'),
	path = require('path');

(function() {
	var checkTree = function(root, dim, leafCellSize) {
		var start = - dim / 2;
		var end = dim / 2;
		
		var file_name = function(x, z, width, height) {
			return "data." + x + "." + z + "."
				+ (x+width) + "." + (z+height);
		}

		for(var cs = leafCellSize ; cs <= dim ; cs <<= 1) {
			for (var z = start ; z < end ; z += cs) {
				for (var x = start ; x < end ; x += cs) {
					var fn = path.join(root, file_name(x, z, cs, cs));
					if (!fs.existsSync(fn))
						throw new Error("File missing: " + fn);
				}
			}
		}
	}

	exports.checkTree = module.exports.checkTreee = checkTree;
})();
