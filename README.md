A set of (experimental) tools to access FITS files from Node.
=========

### To build:

The node-fits module compiles on gnu/linux with g++. node-gyp is used as building tool. Other platforms have not been tested yet.

You will need to install the development versions of libpng and libcfitsio (called libpng12-dev and libcfitsio3-dev on debian systems). 

    #apt-get install node node-gyp g++ libpng-dev libcfitsio3-dev
   
In the node-fits directory, the following commands will build the module:


    $node-gyp configure
    $node-gyp build
    

### Testing

The test.js file, in the node-fits/test directory, loads an example FITS image file (a galaxy), then creates two PNG snapshots of the image using a custom colormap and pixels intensity cut values. 

    $node test.js
    
