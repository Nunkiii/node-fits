//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __PNG_WRITER_HH__
#define __PNG_WRITER_HH__

#include "dcube.hh"
#include <string>

namespace qk{
  void write_png_file(FILE* fp, dcube<unsigned char>& components);
}
#endif
