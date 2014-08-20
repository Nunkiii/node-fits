/*

  node-fits test file.

*/


var fits=require("../build/Release/fits");

var f = new fits.file("example.fits"); //The file is automatically opened for reading if the file name is specified on constructor.


//can also use that way : 
//f.file_name="example.fits";
//f.open(); //Not mandatory


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

	var ab=image.get_data();
	console.log("image data bytes " + ab.length);
	
	

    }

});




console.log("End of script!");
