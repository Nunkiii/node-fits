#include "jpeg_writer.hh"
#include <jpeglib.h>

namespace qk{
  
  using namespace std;
  
  
  void write_jpeg_file(FILE* fp, dcube<unsigned char>& components, float quality){
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
 
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
 
    unsigned int w=components.dims[0];
    unsigned int h= components.dims[1];
      
    cinfo.image_width      = w;
    cinfo.image_height     = h;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    /*set the quality [0..100]  */
    jpeg_set_quality (&cinfo, quality, true);
    jpeg_start_compress(&cinfo, true);


    JSAMPROW row_pointer;          /* pointer to a single row */
 
    while (cinfo.next_scanline < h) {
      row_pointer = (JSAMPROW) components.rowpointer(cinfo.next_scanline);
      jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    

  }
  
}
