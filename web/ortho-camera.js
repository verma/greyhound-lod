// ortho-camera.js
// Ortho camera projections
//

(function(scope) {
	"use strict";

	var OrthoCameras = function(width, height, dist) {
		this.cameras = [];
		this.dist = dist || 100.0;
		for (var i = 0 ; i < 4 ; i ++) {
			this.cameras.push(new THREE.OrthographicCamera(-width/2, width/2, height/2, -height/2));
		}
	};

	OrthoCameras.prototype.updatePosition = function(v) {
		var dist = this.dist;

		for (var i = 0, il = this.cameras.length ; i < il ; i ++) {
			var angle = 2 * Math.PI * i / il;

			var target = new THREE.Vector3(v.x + dist * Math.sin(angle),
										   v.y,
										   v.z + dist * Math.cos(angle))

			this.cameras[i].position.set(v);
			this.cameras[i].lookAt(target);
		}
	}

	OrthoCameras.prototype.resize = function(width, height) {
		for (var i = 0 ; i < 4 ; i ++) {
			this.cameras[i].left = -width/2;
			this.cameras[i].right = width/2;
			this.cameras[i].top = height/2;
			this.cameras[i].bottom = -height/2;

			this.cameras[i].updateProjectionMatrix();
		}
	}

	scope.OrthoCameras = OrthoCameras;
})(window);
