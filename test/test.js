/*

  node-fits test file.

*/


var file_name="example.fits"; //or better: var file_name=process.argv[2]; 
var fits=require("../build/Release/fits"); //This must point to the compiled node-fits plugin.
var f = new fits.file(file_name); //The file is automatically opened (for reading) if the file name is specified on constructor.

var iii=new fits.mat_ushort();
console.log("zero = "+iii.width());
/*

The file can also be specified that way : 

   f.file_name="example.fits";
   f.open(); 


*/

console.log("File is " + f.file_name); //The file_name attribute contains the actually assigned file name.

//Getting the header data from all the hdus present on the fits file

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
	    
            //Get the image binary data into an arraybuffer, for now only as a cast to a float32 arraybuffer, 4bytes/pixel, must be impreved into 
	    //producing typed arraybuffers but lacks v8/ node buffer knowledge for now.
	    
	    var ab=image.get_data();
	    console.log("Image [" + image.width() + ", " +  image.height()+ " ] number of bytes " + ab.length);
	    
	    //... do what you want with the pixels ...
	}
	
    });
  
});
