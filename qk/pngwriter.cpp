#include "pngwriter.hh"

//#define PNG_DEBUG 3
#include <png.h>

namespace qk{

  using namespace std;
  
  
  void write_png_file(FILE* fp, dcube<unsigned char>& components){
    
    int  y;
    
    int width, height;
    png_byte color_type=PNG_COLOR_TYPE_RGBA;
    png_byte bit_depth=8;
    
    png_structp png_ptr;
    png_infop info_ptr;
    //    int number_of_passes;
    //png_bytep * row_pointers;
    
    width=components.dims[0];
    height=components.dims[1];
    
    // int rowbytes=bit_depth*width;
    
    mem<png_bytep> row_pointers(height);
    
    for (y=0; y<height; y++)
      row_pointers.c[y] = components.rowpointer(y);
    
    // char header[8];    // 8 is the maximum size that can be checked
    
    // void write_png_file(char* file_name){
    /* create file */
    //    FILE *fp = fopen(file_name.c_str(), "wb");

    // if (!fp)
    //   throw exception("[write_png_file] File %s could not be opened for writing " + file_name);
    
    
  /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr)
      throw exception("[write_png_file] png_create_write_struct failed");
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
      throw exception("[write_png_file] png_create_info_struct failed");
    
    if (setjmp(png_jmpbuf(png_ptr)))
      throw exception("[write_png_file] Error during init_io");
    
    png_init_io(png_ptr, fp);
    
  
    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
      throw exception("[write_png_file] Error during writing header");
    
    png_set_IHDR(png_ptr, info_ptr, width, height,
		 bit_depth, color_type, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    png_write_info(png_ptr, info_ptr);
    
    
    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
      throw exception("[write_png_file] Error during writing bytes");
    
    png_write_image(png_ptr, row_pointers.c);
    
    
    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
      throw exception("[write_png_file] Error during end of write");
    
    png_write_end(png_ptr, NULL);
    
    //  fclose(fp);
  }
  
}
