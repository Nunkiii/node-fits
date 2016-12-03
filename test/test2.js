var fits=require("../build/Release/fits");

var f=new fits.file();

f.file_name="test.fits";

var M=new fits.mat_ushort();


var B=new Buffer(640*480*2);

for(var i=0;i<640*480;i++){
    var v=i;
    if(i>65535) v=65535;
    B.writeUInt16LE(v,2*i);
}


M.set_data(640,480,B);


console.log("New objects available : ");for (var fm in f) console.log("\t-> " + fm);


f.write_image_hdu(M);

f.set_header_key({ key : "TotoNum", value : 23, comment : "Blblblbl "}, function(error){
    console.log("Error is [" + error + "]");

    f.set_header_key(
	[
	    { key : "TotoString", value : "Hellooooo" , comment : "Blblblbl Bleble"},
	    { key : "TotoString2", value : "Hellooooo Again !" , comment : "Blblblbl Bloblo"},
	    { key : "TotoNum2", value : 3 , comment : "Blblblbl Blala "},
	    { key : "TotoNum3", value : 4 , comment : "Blblblbl Blublu "}
	    
	]
	, function(error){
	    console.log("Error is [" + error + "]");
	});
    
});


