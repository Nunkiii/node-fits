//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __QK_CNT_H__
#define __QK_CNT_H__

namespace qk{
  
  ///Base container class. 
    
  class cnt{
  public:
    cnt():dim(0){}
    virtual ~cnt(){}
    
    ///Returns number of objects contained.
    const int& nel() {return dim;}
    ///Stores the number of objects in the container.
    ///The dim member integer is public but can safely be used for reading only !.
    int dim;
    virtual char* data_pointer()=0;
  };
  
  
}

#endif
