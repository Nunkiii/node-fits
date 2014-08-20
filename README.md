A set of (experimental) tools to access FITS files from Node.
=========

### To build:

The `node-fits` module has been successfully compiled and tested on gnu/linux and on MacOS. node-gyp is used as the building tool. Other platforms have not been tested yet.

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

var fits=require("../build/Release/fits");

var f = new fits.file("example.fits"); //The file is automatically opened for reading if the file name is specified on constructor.


//can also use that way.
//f.file_name="example.fits";
//f.open();


//f.file_name=process.argv[2];

console.log("File is " + f.file_name);



f.get_headers(function(error, headers){
  
  if(error){
    console.log("Bad things happened : " + error);
    return;
  }

  console.log("FITS Headers : " + JSON.stringify(headers,null,5));
  
});


f.read_image_hdu(function(error, image){
    
    if(error){
	console.log("Bad things happened while reading image hdu : " + error);
	return;
    }
    
    if(image){

	console.log("Read image : " + image.width() + ", " + image.height()); 
	
	var colormap=[ [0,0,0,1,0], [1,0,1,1,.8], [1,.2,.2,1,.9], [1,1,1,1,1] ];
	var cuts=[0,200];
	
	image.set_colormap(colormap);
	image.set_cuts(cuts);
	
	var fs=require("fs"),out;

	//The image.tile() function was initially designed for a image viewer's tile generator but can be used to produce
	//custom PNG/JPEG.


	out = fs.createWriteStream("small.png");
	out.write(image.tile( { tile_coord :  [2,3], zoom :  10, tile_size : [64,64], type : "png" }));
	out.end();

	out = fs.createWriteStream("big.jpeg");
	out.write(image.tile( { tile_coord :  [0,0], zoom :  0, tile_size : [512,512], type : "jpeg" }));
	out.end();
	
//	image.histogram({ nbins: 350, cuts : [23,65] }, function(error, histo){ .... 
//      By default cuts are set to min,max and nbins to 200

	image.histogram({}, function(error, histo){ 
	    
	    if(error)
		console.log("Histo error : " + error);
	    else{
		
		console.log("HISTO : " + JSON.stringify(histo));
		
	    }
	});
	
	console.log("End of fits callback!");

        //Get the image binary data into an arraybuffer 

	var ab=image_data.get_data();
	console.log("image data bytes " + ab.length);

    }

});



```
