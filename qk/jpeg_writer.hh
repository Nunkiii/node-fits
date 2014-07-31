//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __JPEG_WRITER_HH__
#define __JPEG_WRITER_HH__

#include "dcube.hh"
#include <string>

namespace qk{
  void write_jpeg_file(FILE* fp, dcube<unsigned char>& components, float quality=90);
}
#endif
