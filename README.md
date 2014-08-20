A set of (experimental) tools to access FITS files from Node.
=========

### To build:

The `node-fits` module has been successfully compiled and tested on gnu/linux and on MacOS. node-gyp is used as the building tool. Other platforms have not been tested yet.

You will need to install the development versions of libpng, libjpeg and libcfitsio. The package names may differ in your distribution. On a debian-based system, the following command should install the required libraries.

    #apt-get install node node-gyp g++ libpng-dev libjpeg-dev libcfitsio3-dev 
   
In the node-fits directory, the following commands will build the module:

    $node-gyp configure
    $node-gyp build
    

### Testing

The test.js file, in the node-fits/test directory, loads an example FITS image file (a galaxy), display all the headers, then creates a PNG and a JPEG snapshots of the image using a custom colormap and pixels intensity cut values. It also shows how to retrieve the image pixel array as a JavaScript ArrayBuffer.

To run the test : 

    $node test.js

It should display headers onthe console and output two images in the test directory.

    
First the package is loaded and a new fits object constructed. The file is automatically opened for reading if the file name is specified on constructor.

      var fits=require("../build/Release/fits");
      var f = new fits.file("example.fits"); 


The file can also be specified as the `file_name` property of the `fits` object. The file is not opened immediately but will be automatically when needed.


    f.file_name="example.fits";


To retrieve the headers, use the `get_headers` function. 

f.get_headers(function(error, headers){
  
  if(error){
    console.log("Bad things happened : " + error);
    return;
  }

  console.log("FITS Headers : " + JSON.stringify(headers,null,5));
  
});


Getting data from an image hdu : 


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

The image.tile() function was initially designed for a image viewer's tile generator but can be used to produce custom PNG/JPEG.


		 out = fs.createWriteStream("small.png");
		 out.write(image.tile( { tile_coord :  [2,3], zoom :  10, tile_size : [64,64], type : "png" }));
		 out.end();

		 out = fs.createWriteStream("big.jpeg");
		 out.write(image.tile( { tile_coord :  [0,0], zoom :  0, tile_size : [512,512], type : "jpeg" }));
		 out.end();
	
//	image.histogram({ nbins: 350, cuts : [23,65] }, function(error, histo){ .... 
By default cuts are set to min,max and nbins to 200

	   image.histogram({}, function(error, histo){ 
	    
		if(error)
			console.log("Histo error : " + error);
	     	else{
		
		console.log("HISTO : " + JSON.stringify(histo));
		
	     	}
	     });
	
Get the image binary data into an arraybuffer :

	      var ab=image_data.get_data();
	      console.log("image data bytes " + ab.length);

	


```
