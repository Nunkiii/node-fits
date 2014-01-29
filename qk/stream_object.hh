#ifndef __STREAM_OBJECT_HH__
#define __STREAM_OBJECT_HH__

//#include <qk3/stream.hh>

namespace qk{

  class stream;

  class stream_object{

  public:
    stream_object(){}
    virtual ~stream_object(){}
    
    ///The io() function actually performs input/ouput operation. Childs have to overload this 
    ///function to read/write their data.
    ///\param _fid The file descriptor of data stream.
    ///\param _dir The direction of transfer (0=read,1=write).

    virtual void io(stream& _s)=0;
    
  };
  



}

#endif
