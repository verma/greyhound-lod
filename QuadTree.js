(function(global) {
	"use strict";

	var TOP_LEFT = 0;
	var TOP_RIGHT = 1;
	var BOTTOM_LEFT = 2;
	var BOTTOM_RIGHT = 3;
	var PARENT = 5;

	function squared(v) { return v * v; }
	function doesCubeIntersectSphere(C1, C2, S, R) {
		var dist_squared = R * R;

		/* assume C1 and C2 are element-wise sorted, if not, do that now */
		if (S.x < C1.x) dist_squared -= squared(S.x - C1.x);
		else if (S.x > C2.x) dist_squared -= squared(S.x - C2.x);
		if (S.y < C1.y) dist_squared -= squared(S.y - C1.y);
		else if (S.y > C2.y) dist_squared -= squared(S.y - C2.y);

		return dist_squared > 0;
	}

	var Node = function(x, y, w, h, d) {
		this.x = x;
		this.y = y;
		this.h = h;
		this.w = w;
		this.depth = d;

		this.children = [];
	};

	Node.prototype.subdivide = function() {
		if (this.depth === 0)
			return;

		var x = this.x;
		var y = this.y;
		var w = this.w;
		var h = this.h;
	

		var d = this.depth;

		this.children.push(new Node(x, y, w/2, h/2, d-1));
		this.children.push(new Node(x+w/2, y, w/2, h/2, d-1));
		this.children.push(new Node(x, y+h/2, w/2, h/2, d-1));
		this.children.push(new Node(x+w/2, y+h/2, w/2, h/2, d-1));

		this.children[0].subdivide();
		this.children[1].subdivide();
		this.children[2].subdivide();
		this.children[3].subdivide();
	}

	Node.prototype.lodQuery = function(eye, lodSpheres, startLOD, callback) {
		var C1 = { x: this.x, y: this.y }, C2 = { x: this.x+this.w, y: this.y+this.h };
		var intersects = doesCubeIntersectSphere(C1, C2, eye, lodSpheres[startLOD]);

		if (!intersects)
			return;

		// if we are already at the bottom of the tree, just emit
		if (startLOD === 0 || this.children.length === 0) {
			console.log('Hit bottom', startLOD, this.children.length)
			console.log(this.depth);
			return callback(this);
		}

		// we got more LOD levels to go, check if a part of this cell
		// intersects a lower LOD level
		var intersectsLower =
			doesCubeIntersectSphere(C1, C2, eye, lodSpheres[startLOD-1]);
		if (!intersectsLower) // doesn't intersect any lower LOD nodes, so return
			return callback(this);

		for (var i = 0 ; i < this.children.length ; i++) {
			var c = this.children[i];

			callback(c); // emit current cell at current depth (or LOD)
			if (doesCubeIntersectSphere({x: c.x, y: c.y}, {x: c.x+c.w, y: c.y+c.h},
				eye, lodSpheres[startLOD-1])) {
				c.lodQuery(eye, lodSpheres, startLOD - 1, callback);
			}
		}
	}

	Node.prototype.collectNodes = function() {
		var thisArr = [this];

		if (this.children.length > 0)
			return thisArr.concat(
				this.children[0].collectNodes(),
				this.children[1].collectNodes(),
				this.children[2].collectNodes(),
				this.children[3].collectNodes());

		return thisArr;
	}

	var QuadTree = function(x, y, w, h, maxDepth) {
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		this.maxDepth = maxDepth;

		var maxDist = Math.sqrt((w * w)+(h * h));
		var LODLevels = [];

		var distF = function(i) { return Math.pow(i, 1/4.0); };

		var maxVal = distF(maxDepth);
		console.log('max val', maxVal);
		for (var i = maxDepth ; i >= 0 ; i--) {
			LODLevels.push(0.09 + 0.91 * (1.0 - (distF(i) / maxVal)));

		}

		console.log(LODLevels);

		this.lodSpheres = [];
		for (var i = 0 ; i < LODLevels.length ; i ++) {
			this.lodSpheres.push(LODLevels[i] * maxDist);
		}

		this.root = new Node(x, y, w, h, maxDepth);
		this.root.subdivide();

		this.entireTree = this.root.collectNodes();
	}

	QuadTree.prototype.lodQuery = function(eyePos, callback) {
		this.root.lodQuery(eyePos, this.lodSpheres, this.lodSpheres.length - 1, callback);
	}

	QuadTree.prototype.LODSpheres = function() {
		return this.lodSpheres;
	}

	QuadTree.prototype.allNodes = function() {
		return this.entireTree;
	}

	global.QuadTree = QuadTree;
})(window);

