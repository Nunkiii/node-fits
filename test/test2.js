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

f.write_image_hdu(M);
