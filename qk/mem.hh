//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __MEM_HH__
#define __MEM_HH__

#define MEM_PROP_NONE 1
#define MEM_PROP_MMAP 2

#include <string.h>
#include "cnt.hh"
#include "stream_object.hh"
#include "stream.hh"

///\file mem.h 
///\brief Basic array container template.

namespace qk{

  using namespace std;


  ///Array class template.  
  ///The mem<T> template class implements a type T data array container.

  template <class T> class mem 
    : public cnt,
    public stream_object{
      
  public:
    ///returns the Read/Write access datapointer : equivalent to use the c member directly. 
    T* data(){return c;}
    
    ///Memory location of data (Read/Write).

  public:
    T * c;
    int prop;

    ///The default constructor. 
    ///\param _dim The initial dimension of the array to allocate. Default is to allocate nothing. 
    
    mem(int _dim=0)    
      :c(NULL),prop(MEM_PROP_NONE){
      redim(_dim);
    }
    
    ///Creates a new array of dimension _dim and initialises its contents from a pointer _o to an array of type T objects. 

    ///Enough data (>= sizeof(T)*_dim bytes) must be already allocated on _o (by stl/gsl vectors for example). 
    ///\param _o A pointer to an array of type T to use as data memory begin position (will be copied to c).
    ///\param _dim The dimension of the input array. 
 
   mem(T * _o, int _dim)    
      :c(_o),prop(MEM_PROP_NONE){
      dim=_dim;
      prop|=MEM_PROP_MMAP;
    }

    ///Copy contructor.

    mem(const mem & _m){
      operator=(_m);
    }

    ///The destructor.

    virtual ~mem(){
      mem_clear();
    }

    virtual char* data_pointer(){ return (char*)c;}
    
    ///Deletes array data.


    virtual void mem_clear(){
      if (c && !(prop&MEM_PROP_MMAP)) delete[] c; //free(c);
      c=NULL;
      dim=0;
      prop=MEM_PROP_NONE;
      return;
    }
    
    ///Returns the data owner status of the array. If the array doesn't own its data, it
    ///won't free it on destruction or resize. 
    ///\return The function returs true if the array owns its data, false otherwise. 

    virtual bool data_own() const{
      return !(MEM_PROP_MMAP&prop);
    }

    ///Resizes the array.
  
    ///\param _dim The new dimension of the array.

    void redim(int _dim){
      
      if(prop&MEM_PROP_MMAP) return;
      if(_dim==dim && c!=NULL) return;
      
      if(dim!=0) mem_clear();

      if(_dim==0) return;
      
      if ( _dim<0){
	MERROR << "Negative dimension! D="<<_dim<<" size "<<sizeof(T)<< " : "<< _dim*sizeof(T)<< " bytes requested." << endl; 
	throw exception("Negative dimension");    
      }

      
      if ((c = new T[_dim]) == NULL) {
	MERROR << "Out of memory ! (asked for "<< _dim*sizeof(T)<< " bytes.)" << endl;
	throw exception("Out of memory");    
      }
      
      dim = _dim;
    }
    

    ///Resizes the array, keeping the existing data. Slow. It is better to a use a \ref olst<T> or
    ///lst<T> if the size of the array needs to be modified without loosing data.
    
    ///\param _dim The new dimention of the array.
    ///\return true on success, false on error. 
    ///\note Desactivated realloc C because of problems ( c=(T*)realloc((void*)c,sizeof(T)*dim) ).
 
    void realloc(int _dim){
      if(_dim==dim) return;      
      if(dim>0){
	if( (c=(T*)realloc(c,_dim*sizeof(T)))== 0){
	  MERROR << "Realloc failed for _dim="<<_dim<<endl;
	  throw exception("Realloc failed");    

	}
	dim=_dim;
	prop=0;
      }else redim(0);
      
    }

    /** 
     * @brief This function is used for mirroring data of type T, already allocated somewhere else at adress _o. 
     * 
     * First, the eventually already allocated data is freed if owned, then the c pointer is
     * simply copied from the value of _o and the dimension dim set to _dim. Finally, the owner flag is
     * set to false so the data won't be freed on destruction by default. 
     *
     * @param _o a pointer to the existing data block.
     * @param _dim the number of elements of type T container in the datablock.
     */
    
    void mmap(T* _o, int _dim){
      mem_clear();
      dim=_dim;
      c=_o;
      prop|=MEM_PROP_MMAP;
    }
    
    /** 
     * @brief Sets all memory bytes to the character byte specified by c. 
     * This just calls the C memset function on the buffer.
     * 
     * @param ch character to be written. By default, the allocated memory is filled with zeros.
     */

    void  memset(int ch=0){
      ::memset((void*)c,ch,dim*sizeof(T));
    }
    
    /**                                                                                                                                                    
     *  @brief  Access to the data contained in the array.                                                                                     
     *  @param index The index of the element
     *  @return  Read/write reference to data element.
     *                                                                                                                                                     
     *  Access is NOT checked for bad indexes. 
     */

    
     T & operator [](int index){
       return c[index];
     }
     
     /**                                                                                                                                                    
      *  @brief  Access to the data contained in the array.                                                                                     
      *  @param index The index of the element
      *  @return  Read only reference to data element.
      *                                                                                                                                                     
      *  Access is NOT checked for bad indexes. 
      */
     
     
     const T & operator [](int index) const {
       return c[index];
     }


    /**                                                                                                                                                    
     *  @brief  Access to the data contained in the array.                                                                                     
     *  @param index The index of the element
     *  @return  Read/write reference to data element.
     *                                                                                                                                                     
     *  Access is always checked for bad indexes. 
     */

    
    T & operator ()(int index) const{
       if(check_index(index)) 
	 return c[index];
       //MERROR << "Array index "<< index <<" out of bounds. ("<< dim <<")" << endl;
       throw exception("mem<T> : Index out of bounds !");    
       
    }
    
    // /**                                                                                                                                                    
    //   *  @brief  Access to the data contained in the array.                                                                                     
    //   *  @param index The index of the element
    //   *  @return  Read only reference to data element.
    //   *                                                                                                                                                     
    //   *  Access is always checked for bad indexes. 
    //   */
    
    
    // const T & operator ()(int index) const {
    //   if(check_index(index))
    // 	return c[index];
    //   MERROR << "Array index "<< index <<" out of bounds. ("<< dim <<")" << endl;
    //   throw exception("Index out of bounds");    
    // }
    

     /**                                                                                                                                                    
      *  @brief  Array copy. 
      *  @param _m Array to copy.
      *  @return  Read only reference to this array object.
      *                                                                                                                                                     
      *  No comment.
      */
     
     const mem<T> & operator=(const mem<T> & _m){
       redim(_m.dim);
       memcpy(c, _m.c, sizeof(T)*dim);
       return *this;
     }
     
     
     /**                                                                                                                                                    
      *  @brief  Array copy from existing datapointer. 
      *  @param _m Data pointer from where to copy data.
      *  @param _dim Number of elments of type T to read. If not set (the default), then 
      *  the current array size of elements will be read.
      *  @return  Read only reference to this array object.
      *                                                                                                                                                     
      *  Enough data should be available at given _c location. 
      */
     
     const mem<T> & rawcopy(T * _c, int _dim=-1){
       if(_dim!=-1) redim(_dim);      
       memcpy(c, _c, sizeof(T)*dim);
       
       return *this;
     }
     
     /**                                                                                                                                                    
      *  @brief  Copy a sub-array from another array.
      *  @param _m Array to copy data from.
      *  @param _start Index of first element to copy.
      *  @param _n Number of elements to copy. If not set (the default), then 
      *  the current array size of elements will be read.

      *  @return  Read only reference to this array object.
      *  
      */
     
     const mem<T> & bloc_copy(mem<T> & _m, int _start=0, int _n=-1){
       return rawcopy(_m.c+_start, _n);
     }
     

     /**                                                                                                                                                    
      *  @brief Array comparison.
      *  @param _m Array to copy data from.

      *  @return  True if arrays have the same memory content, false otherwise.
      *  
      */

     bool operator == (const mem & _b) const{
       
       if (this == &_b) return true;
       if (_b.dim!=dim) return false;
       
       if( bcmp(_b.c,c,dim*sizeof(T)) != 0) return false;
       
       return true;
     }
     
     bool operator != (const mem & _b) const{ return !operator==(_b);}
     
     virtual void io(stream& _s){
       int __dim=dim;
       
       _s.IO(__dim);
       if(!_s.dir()) redim(__dim);
       
       if(dim!=0)
	 _s.io( (void*)c, dim*sizeof(T));
     }
    
    bool check_index(const int _index) const{
       return (_index>=0 && _index<dim);
     }

    bool check_dim(const mem<T>& _b) const{
       return _b.dim==dim;
     }
     
  };
  

  ///Array of pointers. Adds memory management.

  template <class T> class pmem : public mem<T>{
  public:
    
    using mem<T>::dim;
    using mem<T>::redim;
    using mem<T>::c;
    using mem<T>::prop;
    

    ///if dp is true, the array will delete its non-null objects whn destroyed.
    bool dp;
    virtual bool data_own(){return dp;}

    ///Initialises the array. The array is initially filled with NULL pointers.

    virtual void mem_create(int _nel){
      redim(_nel);
      for(int i=0;i<dim;i++)c[i]=NULL; 	
      return;
    }
  

    pmem(int _dim=0):mem<T>(_dim),dp(true){}
    pmem(T * _o, int _dim):mem<T>(_o,_dim),dp(true){}
    pmem(const mem<T> & _m):mem<T>(_m),dp(true){}
    virtual ~pmem(){mem_clear();}

    ///Frees allocated memory.

    virtual void mem_clear(){
      if (c && !(prop&MEM_PROP_MMAP)){

	if(dp){
	  int index;
	  for(index=0;index<dim;index++)
	    if(c[index]!=NULL)delete c[index];
	}

	delete[] c; //free(c);
      }
      c=NULL;
      dim=0;
      prop=MEM_PROP_NONE;
      return;
    }

    ///Removes an element from array (dim is decreased by one). Slow.
    ///\param _i index of array element to remove.
    ///\param _deep If true, each pointer array element is also freed. 
    
    virtual void retire(int _i, bool _deep=1){

      if(_i<0||_i>=dim){
	cerr << "Invalid index " << _i << " D="<<dim<<endl;
	throw exception("Index out of bounds");
      }

      int index;

      pmem<T> cc(*this);
      redim(dim-1);

      for(index=0;index<_i;index++)
	c[index]=cc[index];
      for(index=_i+1;index<dim;index++)
	c[index-1]=cc[index];
      
      if(dp||_deep) delete cc[_i];

    }
  };
  
  
}
#endif

