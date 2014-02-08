// downloadWorker
// Download a tile and notify when complete
//

self.addEventListener('message', function(e) {
	var node = e.data;

	console.log(node);
	var url = (function(d) {
		return "/points/cloud/"
			+ node.x + "/"
			+ node.z + "/"
			+ node.width + "/"
			+ node.height;
	})();
	var oReq = new XMLHttpRequest();
	oReq.open("GET", url, true);
	oReq.responseType = "arraybuffer";

	oReq.onload = function(oEvent) {
		if (oReq.status == 200) {
			var data = oReq.response;
			oReq = null;

			var floatsInRecord = 7;
			var recordSize = floatsInRecord*4;

			console.log(data.byteLength, data.byteLength / recordSize);

			var floatArray = new Float32Array(data);

			var pointCount = floatArray.length / floatsInRecord;
			var pos = new Float32Array(3 * pointCount);
			var col = new Float32Array(3 * pointCount);
			var intensity = new Float32Array(1 * pointCount);

			var si = 0;
			for (var i = 0, il = pos.length ; i < il ; i += 3, si+=7) {
				pos[i] = floatArray[si];
				pos[i+2] = floatArray[si+1];
				pos[i+1] = floatArray[si+2]; // don't touch the height

				col[i] = floatArray[si+3];
				col[i+1] = floatArray[si+4];
				col[i+2] = floatArray[si+5];

				intensity[i/3] = floatArray[si+6];

			}

			// transfer node and downloaded array buffer to
			// main thread
			self.postMessage({
				position: pos, color: col, intensity: intensity },
				[pos.buffer, col.buffer, intensity.buffer]);
		}
	};

	oReq.send();
}, false);
