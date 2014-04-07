# What is this?
This is a highly experimental territory.  Prepare yourself for high levels of frustration and uncontrollable rage and disgust towards me.  With that in mind lets move on.

This is an experimental project based off of the Grehound family of technologies.  This demo shows how to stream and render large amounts of point cloud data in a browser.  The data is arranged as a quad-tree on the server and is fed to the client over HTTP `GET` requests.

Since the client side of this experiment is very demanding data-wise, the data is pre-computed and staged on the server.  This reduces runtime computational overhead and makes data suitable for deployment on a static CDN.

# Directory Structure
The root directory of this project has a few sub-directories:

- `tools` - This is a collection of C++ and JS tools written to help stage and validate pre-computed data.
- `playground` - Experimental stuff inside experimental stuff.
- `web` - The webserver which runs the actual demo server.
- `scripts` - Vagrant related stuff.

## The `tools` Directory
The tools directory is only Make-able under the provided Vagrant configuration.  The programs do the following:

- `fetch-terrain.cpp` - Given a whole bunch of command line arguments, this function queries the database for data through PDAL and dumps it out into an appropriately named file.  For some reason this code cannot be called in loops (it crashes), so another script was written to do the logistics of managing this program (discussed next).
- `call-fetch-terrain.js` - This script is responsible for calling the program above.  You can configure some vars in it like `terrainSize` and `leafSize`.  Also, please make sure that `getDataDimensions` function is doing what you want it to.
- `gen-large-terrain.cpp` - Not used anymore.  This was used to generate procedural terrain using a perlin noise function.
- `tail-points.cpp` - Something similar to the unix `tail` command, just to print points from a quad-tree node to make sure they make sense.
- `mipmap-terrain.cpp` - When you have the leaf-nodes of the quad-tree ready (after calling `call-fetch-terrain.js`) you run this on the data directory to generate higher quad-tree nodes.
- `query-raw.cpp` - Checks if any points in the given area are all zeros.  Not sure why this was written.
- `test-pdal-db.js` - Used to test connection to PDAL and the DB Server.  I think this program is also used to determine the dimensions of data (precompute them and use them directly).
- `gzip-compress-data.js` - To be used after all data has been generated.  An optional tool to reduce the size of generated quad-tree nodes.  All nodes are GZIP compressed.  Since browsers support GZIP data decompression, we can directly stream GZIP files to the browser which takes care of decompressing them.
- `check-zeros.js` - Runs and manages the `query-raw.cpp` program.

## The `playground` Directory
This directory is for test programs or algorithms.  The program of interest here is `quad.html` which demonstrates how the LOD algorithm works.  The other HTML file called `render-boxex.html` just renders the PDAL chipper chips (the data is static and defined inside the file, for brave text-editors only).

## The `scripts` Directory
Vagrant specific scripts.

## The `web` Directory
This is the home of the server which runs the demo and serves data to it.  It looks for the `data` directory in the projects home directory (relative path `../data`), validates it and serves everything on port `8000`.

You don't have to have your data GZIPed for this to work, just that you have to pass `NOGZIP=1` environment flags to the program to tell it not assume GZIPed data.


# How to run this?
## Generating files

Make sure you have point cloud data available somewhere which PDAL can access.  Connect to your vagrant instance.

    vagrant up
    vagrant ssh

Build the tools.

    cd tools
    make

Once the tools are ready, go back to the projects root directory and run call-fetch-terrain.js

    cd ..
    npm install
    node tools/call-fetch-terrain.js
    
This tool takes several command line options.  E.g. if you notice that some of your files are of zero length (this can happen when you computer sleeps while generating data), you can rebuild missing files:

    node tools/call-fetch-terrain.js rebuild
    
If you want to parallelize data generation, you can do this:

    node tools/call-fetch-terrain.js 0 0
    
The `0 0` argument tells the program to compute and generate data for upper left quadrant, `0 1` for upper right etc.  You could run 4 instances of this program which parameters `0 0`, `0 1`, `1 0` and `1 1` which will churn data out in parallel for the 4 quads.

The data will be written to the `data` directory.  You now need to generate mipmaps:

    tools/mipmap-terrain 16 8
    
Here `16` is the size of the terrain (as power of two, also specified in `call-fetch-terrain.js`), and `8` is the leaf-size (256x256).

Once this is done, you can optionally compress data.

    tools/glib-compress-data.js
    
At this point you should be ready to serve data.
  

## Serving Files

Go to the `web` directory and run the server program.

    cd web
    node server.js
    
If you did not GZIP compress your data, you will have to run the server like:

    NOGZIP=1 node server.js
    
    
    
The output should look something like:
    
    Data source specs:
    terrrain size: 32768
        leaf size: 256
    Data tree looks good.
    Listening on port: 8000

Now visit `http://localhost:8000/` and you should be good to go.

# Terrain and Leaf Sizes
You specify terrain and leaf sizes as powers of two (for simplicity).  Whatever your data's dimensions may be, the coordinates are scaled to fit them into the requested terrain size without affecting the aspect ratio.

This sometimes causes the points to become too sparse. General principal is to keep the scale as close to 1.0 as possible.  The `call-fetch-terrain.js` program reports this value when its run.

The Leaf size is again represented as a power of two and the difference between the terrain and leaf sizes determines how many Levels of Detail there are going to be.

# Issues
There may be issues with the process above (since its been a while since I ran this), if so please raise an issue and we'd take care of it.