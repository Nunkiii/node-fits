//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __STREAM_HH__
#define __STREAM_HH__


#include <errno.h>
#include <string>
#include <iostream>
//#include <string>

#include "ansi.h"
#include "exception.hh"
//#include "class.hh"

//#include <qk3/stream_object.hh>


namespace qk{

    
#define QK_READ 0
#define QK_WRITE 1
#define QK_DELETE 2
#define QK_WRITE_REQUEST 3
  
  typedef long long int64;
  typedef long int32;
  typedef short int16;
  typedef char int8;
  typedef unsigned long long uint64;
  typedef long long int64;
  typedef unsigned long uint32;
  typedef unsigned short uint16;
  typedef unsigned char uint8;
  
#define DEFAULT_TMP_FILE_SIZE 16
  
  class object;
  
  //  class stream_object;
  
  template<class T> inline T max(T a, T b) { return (a < b) ? b : a; }
  //  enum io_dir { QK_READ, QK_WRITE }; 
  
  /// The SS_OPENING state indicates that the stream will signal open or closed
  /// in the future.
  
  enum stream_state { SS_CLOSED, SS_OPENING, SS_OPEN };
  
  /// Stream read/write methods return this value to indicate various success
  /// and failure conditions described below.
  enum stream_result { SR_ERROR, SR_SUCCESS, SR_BLOCK, SR_EOS, SR_TIMEOUT };
  
  /// StreamEvents are used to asynchronously signal state transitionss.  The flags
  /// may be combined.
  ///  SE_OPEN: The stream has transitioned to the SS_OPEN state
  ///  SE_CLOSE: The stream has transitioned to the SS_CLOSED state
  ///  SE_READ: Data is available, so Read is likely to not return SR_BLOCK
  ///  SE_WRITE: Data can be written, so Write is likely to not return SR_BLOCK
  enum StreamEvent { SE_OPEN = 1, SE_READ = 2, SE_WRITE = 4, SE_CLOSE = 8 };
  
  ///The two access mode : for reading or writing. 
  enum access_mode { Read, Write };


  ///Basic stream class for IO operations.

  class stream{
  public:
    stream():
      stream_properties(0),
      bcount_total(0){}
    virtual ~stream(){}

    
    virtual void buffer_io(void* _buffer, int _buffer_len, int* _bcount=0) = 0;

    void io(void* _buffer, int _buffer_len, int* _bcount=NULL);
    
    virtual stream_state get_state() const = 0;
    
    /// Attempt to transition to the SS_CLOSED state.  SE_CLOSE will not be
    /// signalled as a result of this call.
    virtual void close() = 0;
    
    /// Return the number of bytes that will be returned by \refer buffer_io in read mode,
    ///if known.
    virtual bool get_size(int* _size) const = 0;
    
    // Communicates the amount of data which will be written to the stream.  The
    // stream may choose to preallocate memory to accomodate this data.  The
    // stream may return false to indicate that there is not enough room (ie, 
    // Write will return SR_EOS/SR_ERROR at some point).  Note that calling this
    // function should not affect the existing state of data in the stream.
    virtual bool reserve_size(int _size) = 0;
    
    /// Returns true if stream could be repositioned to the beginning.
    virtual void rewind(){ throw exception("rewind not implemented");} 
    virtual bool eof() const = 0;
    

    virtual void set_position(long int _position){ throw exception("set_position not implemented");}
    virtual long int get_position() const{ throw exception("get_position not implemented");}
    
    
    ///Sets the direction of the flow on this stream: 0 for reading, 1 for writing.

    void set_direction(int _dir){ stream_properties=_dir; }

    ///Reads stream until an end of line character is found. 
    void read_line(std::string* _line);
    void read_all(std::string& _line);


    void io_ascii(const std::string& _text){
      io((void*)_text.c_str(),_text.size());
    }
    
    stream& operator << (const std::string& ascii_data) {  set_direction(1), io_ascii(ascii_data); return *this; }

    stream& operator << (const int& value) {  set_direction(1), IO(value); return *this; }
    stream& operator << (const double& value) {  set_direction(1), IO(value); return *this; }
    stream& operator << (const float& value) {  set_direction(1), IO(value); return *this; }
    
    template <class Type> inline void IO(Type& _object){
      io((void*)&_object,sizeof(Type));
    }
    
    void read_class_info(int& mclid, int& clid);
    void write_class_info(int mclid, int clid);


    bool dir() const {return stream_properties&QK_WRITE;} 
    
    
    int stream_properties;
    int bcount_total;
  };


  void create_random_string(int _nchar, char* _s);
  void create_tmp_file(std::string& _tmp_file, int _nchar=DEFAULT_TMP_FILE_SIZE);

  std::string create_random_file(const std::string& prefix="", const std::string& suffix="", int _nchar=DEFAULT_TMP_FILE_SIZE);


  void get_stream_class_info(object& obj, int& mclid, int& clid);
  void elm_copy(object& _src, object& _dest);
  object* elm_create_copy(object& _src);  
  int open_file(const std::string& _file, bool _dir);

  void flow(stream* _source,
		     char* _buffer, int _buffer_len,
		     stream* _sink);
  
  
}

#endif
