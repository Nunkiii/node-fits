//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __DCUBE_HH__
#define __DCUBE_HH__

#include "mat.hh"

namespace qk{

  template <class T>
  class dcube: public vec<T>{
  public:

    using vec<T>::c;
    int dims[3];

    dcube(){}

    dcube (int _d0, int _d1, int _d2){
      redim(_d0,_d1,_d2);
    }    
    
    ///Constructor.
    
    dcube (int* _dims)  
      :vec<T>(0){
      redim(_dims[0],_dims[1],_dims[2]);
    }    

    ///Copy constructor.

    dcube (const dcube & m){
      operator = (m);
    }   

    ///Resizes the image.
    ///\param _d0: the new row dimension.
    ///\param _d1: the new column  dimension.

    virtual void redim(int d0, int d1, int d2){
      mem<T>::redim(d0*d1*d2);
      dims[0] = d0;
      dims[1] = d1;
      dims[2] = d2;
    }

    ///Resizes the image.
    ///\param _dims: A pointer to a (at least 2) sized array containing the new dimensions.
    void mredim(int * _dims) {redim(_dims[0], _dims[1], _dims[2]); }

    void mmap(T* _data, int _d0, int _d1, int _d2){
      mem<T>::mmap(_data,_d0*_d1*_d2);
      dims[0]=_d0;
      dims[1]=_d1;
      dims[2]=_d2;
    }
    
    ///Destructor.
    
    virtual ~dcube (){}

    ///Equality operator. 
    ///\param _ms : The matrix to copy data from. 

    const dcube<T> & operator = (const dcube<T> & ms){
      if (this != &ms){
	dims[0] = ms.dims[0];
	dims[1] = ms.dims[1];      
	dims[2] = ms.dims[2];      
	vec<T>::operator=(ms);
      }    
      return *this;
    }

    T* rowpointer(int row){
      return &c[row*(dims[0]*dims[2])]; 
    }
    
    void io(stream& _s){
      
      _s.io((char*)dims, sizeof(int)*3);
      vec<T>::io(_s);
    }


    ///Checks dimension compatibility with another matrix.

    bool check_dims(const dcube<T>& _m) const{ return (dims[0]==_m.dims[0] && dims[1]== _m.dims[1] && dims[2]== _m.dims[2]) ;  }


    ///Returns true if pixel row y, column x, cell z falls inside image.

    inline bool is_in(int _x, int _y, int _z) const{
      if(_y<0||_y>=dims[1] || _x<0||_x>=dims[0] || _z<0||_z>=dims[2])
	return false;
      return true;
    }
    

    ///Returns the value of the matrix element contained in the row _x, and column _y.
    ///\param _y : the index of the column.
    ///\param _y : the index of the row.
    ///If the specified index pair falls outside the matrix dimensions, an exception is thrown.

    inline T& value(int _y, int _x, int _z){ 
      if(!is_in(_x,_y,_z)){
	MERROR << "Value ("<<_x<<","<<_y<<") out of bounds. dimensions :  " << dims[0] << ", " << dims[1] << ", "<< dims[2]<<endl;
	throw exception("indexed out of bounds");
      }
      return c[_y*(dims[0]*dims[2])+_x*dims[2]+_z]; 
    }

    ///Returns the value of the matrix element contained in the row _x, and column _y. const version.
    ///\param _y : the index of the column.
    ///\param _y : the index of the row.
    ///If the specified index pair falls outside the matrix dimensions, an exception is thrown.

    inline const T& value(int _y, int _x, int _z) const { 
      if(!is_in(_x,_y,_z)){
	MERROR << "Value ("<<_x<<","<<_y<<") out of bounds. dimensions :  " << dims[0] << ", " << dims[1] <<endl;
	throw exception("indexed out of bounds");
      }
      return c[_y*(dims[0]*dims[2])+_x*dims[2]+_z]; 
    }
    


    ///Access a matrix element specified by its row (y)
    ///and column (_x). The order (y,x) recalls that data is stored in row major order. 
    ///No boundary check is performed for the indices for performance concerns. See \ref value() for safe, checked retrieval.

    inline T & operator () (int _y, int _x, int _z){
      return c[_y*(dims[0]*dims[2])+_x*dims[2]+_z]; 
    }

    ///Access a matrix element specified by its row (y)
    ///and column (_x). The order (y,x) recalls that data is stored in row major order. 
    ///No boundary check is performed for the indices for performance concerns. See \ref value() for safe, checked retrieval.

    inline const T & operator () (int _y, int _x, int _z) const {
      return c[_y*(dims[0]*dims[2])+_x*dims[2]+_z]; 
    }


    template <class MT> void get_matrix(mat<MT>& m){
      m.redim(dims[0],dims[1]);
      m.memset(0);
      int i,j,c;
      for(i=0;i<dims[1];i++)
	for(j=0;j<dims[0];j++)
	  for(c=0;c<dims[2];c++)
	    m(i,j)+=(MT)operator()(i,j,c);
    }

    template <class MT> void get_matrix(unsigned int component, mat<MT>& m){
      m.redim(dims[0],dims[1]);      
      int i,j;
      for(i=0;i<dims[1];i++)
	for(j=0;j<dims[0];j++)
	  m(i,j)=(MT)operator()(i,j,component);
      
    }




  };
  
}

#endif
