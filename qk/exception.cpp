//#include "object.hh"
#include "exception.hh"

namespace qk {
  
  exception::exception(const std::string& me, int info) throw()
    : mess(me), ex_info(info){
  }
  
  exception::~exception() throw()
  {}
  
  const char* exception::what() const throw(){
    return mess.c_str();
  }
  
  void exception::raise() const{
    throw *this;
  }

  /*  
  io_error::io_error(const std::string& me) throw() : exception("io error : " + me){}  
  io_error::~io_error() throw(){}  
  void io_error::raise() const{throw *this;}

  access_error::access_error(const std::string& me) throw() : exception("Access error : ["+ o->get_info()+"]"+me){}  
  access_error::~access_error() throw(){}  
  void access_error::raise() const{throw *this;}
  */
}
