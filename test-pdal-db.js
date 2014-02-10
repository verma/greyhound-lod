var pg = require('pg'); 
var fs = require('fs');
//or native libpq bindings
//var pg = require('pg').native

var conString = "postgres://howardbutler@localhost:5432/lidar";

var terrainSize = 1 << 14;
var leafSize = 1 << 7;

function intersectRect(r1, r2) {
  return !(r2.left > r1.right || 
           r2.right < r1.left || 
           r2.top > r1.bottom ||
           r2.bottom < r1.top);
}

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
console.log("Connecting ...");
client.connect(function(err) {
  if(err) {
    return console.error('Could not connect to postgres', err);
  }

  console.log("... Connected.")
  client.query("select Box2D(pa::geometry) from lidar", function(err, res) {
	  if (err) return console.log(err);

	  var dataExtents = {
		  left: -10328630.5337897,
		  top: 9400741.04756276,
		  right: -10293814.8637897, 
		  bottom: 9433270.07756276 };

	  var boxes = [];
	  for (var i = 0 ; i < res.rows.length ; i ++) {
		  boxes.push(parseBox(res.rows[i].box2d));
	  }

	  console.log('Total', boxes.length, 'boxes');
	  fs.writeFileSync("boxes.json", JSON.stringify(boxes));
	  console.log('File written');



	  var dims = [dataExtents.right - dataExtents.left, dataExtents.bottom - dataExtents.top];
	  var squareDim = Math.max(dims[0], dims[1]);

	  var center = {
		  x: dataExtents.left + (dataExtents.right - dataExtents.left),
		  y: dataExtents.top + (dataExtents.bottom - dataExtents.top)
	  };

	  console.log(dims, squareDim);

	  var leafDims = [squareDim / leafSize, squareDim / leafSize];
	  console.log("Area per leaf:", leafDims[0], 'x', leafDims[1]);

	  for (var y = 0 ; y < terrainSize ; y += leafSize) {
		  for (var x = 0 ; x < terrainSize  ; x += leafSize) {
			  var xf = 2 * x / terrainSize - 1.0;
			  var yf = 2 * y / terrainSize - 1.0;

			  var xs = center.x + squareDim * xf;
			  var ys = center.y + squareDim * yf;

			  var xe = xs + leafDims[0];
			  var ye = ys + leafDims[1];

			  if (intersectRect(dataExtents, {
				  left: xs, top: ys,
				  right: xe, bottom: ye
			  })) {
				  //console.log(xf, yf, xs, ys, xe, ye);
			  }

		  }
	  }
  });

  /*
  client.query('SELECT ST_Union(f.extents) FROM(SELECT 

  client.query('SELECT schema FROM pointcloud_formats WHERE pcid = 1', function(err, res) {
	  if (err) return console.log('Error fetching schema:', err);
	  
	  console.log(res.rows[0].schema);
  });

  client.query('SELECT Sum(PC_NumPoints(pa)) as "totalPoints" FROM lidar', function(err, result) {
    if(err) {
      return console.error('Error running query:', err);
    }

    console.log("Total points in database:", result.rows[0].totalPoints);
    client.end();
  });
  */
});
