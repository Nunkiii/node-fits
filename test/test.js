/*

  node-fits test file.

*/


var fits=require("./build/Release/fits");

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
