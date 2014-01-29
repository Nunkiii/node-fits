A set of (experimental) tools to access FITS files from Node.
=========

### To build:

The node-fits module is compiling on gnu/linux with g++. node-gyp is used as building tool. Other platforms have not been tested.

You will need to install the developpement versions of libpng and libcfitsio (called libpng12-dev and libcfitsio3-dev on debian systems). 

    #apt-get install node node-gyp g++ libpng-dev libcfitsio3-dev
   
In the node-fits directory, the following commands should build the module:


    $node-gyp configure
    $node-gyp build
    

### Testing

The test.js file in the node-fits/test directory loads an example FITS image file (a galaxy), then creates two PNG snapshots of the image data using a custom colormap and value cuts. 

    $node test.js
    
