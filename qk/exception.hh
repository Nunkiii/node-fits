//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __EXCEPTION_HH__
#define __EXCEPTION_HH__

#include <exception>
#include <string>

namespace qk
{
  
  class object;

  /** Base class for all qk exceptions.
   */
  class exception: public std::exception
  {
  public:
    explicit exception(const std::string& message, int info=0) throw();
    virtual ~exception() throw();
    
    virtual const char* what() const throw();
    virtual void raise() const;
    //  private:
    std::string mess;
    int ex_info;
  };
  /*
  class  io_error: public exception{
  public:
    explicit io_error(const std::string& message) throw();
    virtual ~io_error() throw();
    virtual void raise() const;
  };

  class  access_error: public exception{
  public:
    explicit access_error(const std::string& message) throw();
    virtual ~access_error() throw();
    virtual void raise() const;

    object* o;
  };
  */

  
} 

#endif 


