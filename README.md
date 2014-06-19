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


    File test.js

```
/*

  node-fits test file.

*/


var fits=require("../build/Release/fits");

var f = new fits.file();

f.file_name="example.fits";

f.read_image_hdu(function(error, image){
    
    if(error) console.log("Bad things happened : " + error);
    
    if(image){

	//var headers=f.get_headers(); console.log("FITS headers : \n" + JSON.stringify(headers, null, 4));
	
	var colormap=[ [0,0,0,1,0], [1,0,1,1,.8], [1,.2,.2,1,.9], [1,1,1,1,1] ];
	var cuts=[0,200];
	
	image.set_colormap(colormap);
	image.set_cuts(cuts);
	
	var fs=require("fs"),out;

	out = fs.createWriteStream("small.png");
	out.write(image.gen_pngtile([0,0,0], [64,64]));
	out.end();

	out = fs.createWriteStream("big.png");
	out.write(image.gen_pngtile([0,0,0], [512,512]));
	out.end();

    }

});


```
