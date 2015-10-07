/*

  node-fits test file. Written by: Pierre Sprimont <sprimont@iasfbo.inaf.it>

*/

var file_name="example.fits"; //or, more usefull: var file_name=process.argv[2]; 

//Including the fits plugin. The path must point to a compiled node-fits plugin.

var fits=require("../build/Release/fits"); 

//Shows all the new JS 'classes' available to the user :
console.log("New objects available : ");for (var fm in fits) console.log("\t-> " + fm);

var f = new fits.file(file_name); //The file is automatically opened (for reading) if the file name is specified on constructor.

var iii=new fits.mat_ushort();
//console.log("zero = "+iii.width());
//for (var p in iii) console.log("IMPROP : " + p);
console.log("matrix OK");

/*

The file could also be specified that way and opened 'manually' 

   f.file_name="example.fits";
   f.open(); 


*/

console.log("File is " + f.file_name); //The file_name attribute contains the actually assigned file name.

//Getting the header data from all the data-units present on the fits file

f.get_headers(function(error, headers){
    
    if(error){
	console.log("Bad things happened : " + error);
	return;
    }
    
    console.log("FITS Headers : " + JSON.stringify(headers,null,5));

    
    //Reading an image as an arraybuffer of floats (4bytes/pixel). The returned image is another JS imported C++ class representing 2D data organised by rows: jsmat class. 
  //(this has to be extended to extract the data in other javascript supported binary types)
  
    f.read_image_hdu(function(error, image){


	
	if(error){
	    console.log("Bad things happened while reading image hdu : " + error);
	    return;
	}
	
	if(image){

	    //for (var ip in image) console.log("IP : " + ip);
	    
	    console.log("Image size : " + image.width() + " X " + image.height()); 
	    
	    var colormap=[ [0,0,0,1,0], [1,0,1,1,.8], [1,.2,.2,1,.9], [1,1,1,1,1] ];
	    var cuts=[20,1500];
	    
	    image.set_colormap(colormap);
	    image.set_cuts(cuts);
	    
	    var fs=require("fs"),out;
	    
	    //The image.tile() function was initially designed for a image viewer's tile generator but can be used to produce
	    //custom PNG/JPEG.
	    
	    out = fs.createWriteStream("small.png");
	    out.write(image.tile( { tile_coord :  [0,0], zoom :  0, tile_size : [64,64], type : "png" }));
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
	    
            //Get the image binary data into an arraybuffer, for now only as a cast to a float32 arraybuffer, 4bytes/pixel, must be impreved into 
	    //producing typed arraybuffers but lacks v8/ node buffer knowledge for now.
	    
	    var ab=image.get_data();
	    console.log("Image [" + image.width() + ", " +  image.height()+ " ] number of bytes " + ab.length);

            console.log("First pix is " + ab[0]);
      
	    //... do what you want with the pixels ...
	}
	
    });
  
});
