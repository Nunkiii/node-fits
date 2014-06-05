//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "vec.hh"

namespace qk{


  ///Basic rectangle template.
  
  ///A rectangle is based on a vec<T> type with a fixed number of 4 elements.
  /// -- The two first represents the x and y coordinates of the area's top/left corner.
  /// -- The last two components contains the size of the width and height of the rectangle.

  template <class T> class rect: 
    public vec<T>{

  public:
  
    using mem<T>::c;
    using mem<T>::dim;
    
    ///Constructor. Array is resized to 4.
    
    rect():vec<T>(4){}
    rect(T x,T y, T w, T h):vec<T>(4){c[0]=x;c[1]=y;c[2]=w;c[3]=h;}
    
    virtual ~rect(){}
    ///Returns the width of the rectangle.
    inline T & L() { return c[2]; }
    ///Returns the height of the rectangle.
    inline T & l() { return c[3]; }
    ///Returns the width of the rectangle.
    inline T & w() { return c[2]; }
    ///Returns the height of the rectangle.
    inline T & h() { return c[3]; }
    ///Returns the x coordinate of the rectangle's top/left corner.
    inline T & x() { return c[0]; }
    ///Returns the y coordinate of the rectangle's top/left corner.
    inline T & y() { return c[1]; }
  
    ///Returns true if the rectangle specified by _rect is included into the rectangle's area. 
    bool rect_include(const rect<T>& _rect){
      for(int i=0;i<2;i++)
	if(_rect[i]<c[i] || _rect[i+2]+_rect[i]>c[i]+c[i+2])
	  return false;
      return true;
    }
    
    ///Retrieves the intersection of the rectangle with the rectangle given by _rect. 
    /// -- If the intersection exists, it is returned by the input _sect rectangle and the function returns true.
    /// -- Of the intersection is empty, the function returns false and the vaue of _sect is undefined.

    bool rect_isect(const rect<T> & _rect, rect<T> & _sect){
      for(int i=0;i<2;i++){
        if(_rect[i]<c[i] && (_rect[i]+_rect[i+2])>=c[i]){
  	  _sect[i]=c[i];
  	  _sect[i+2]= ((_rect[i]+_rect[i+2])<(c[i]+c[i+2])) ? _rect[i+2]-c[i]+_rect[i] : c[i+2];
  	}
        else
	  if(_rect[i]>=c[i] && _rect[i]<(c[i]+c[i+2])){
  	    _sect[i]=_rect[i];
  	    _sect[i+2]= ((_rect[i]+_rect[i+2])<(c[i]+c[i+2])) ? _rect[i+2] : (c[i]+c[i+2]-_rect[i]);
  	  }
	  else
	    return false;
      }
      
      return true;
    }
    
    
    
    ///Returns true if the point lies into the rectangle's area, false otherwise.
    bool is_point(T * _point){
      for(register int i=0;i<2;i++)
	if(_point[i]<c[i] || _point[i]>c[i]+c[i+2])
        return false;
      return true;
    }
    
    
    ///Expands the area of the rectangle so taht it includes the rectangle given by _rect.
    void rect_expand(const rect<T>& _rect){
      for(int i=0;i<2;i++){
	if(_rect[i]<c[i])
	  c[i]=_rect[i];
	if((_rect[i+2]+_rect[i])>(c[i]+c[i+2]))
	  c[i+2]=_rect[i]+_rect[i+2]-c[i];
      }      
    }
    
    ///Retrieves the coordinates from a rectangle frame on the local rectangle frame.
    template <class U> bool get_coord(U* _c, const rect<U>& _cbase, T* _local_c,
				      bool _invertx=false, bool _inverty=false){
      //      U cc[2];

      _local_c[0]=(T)(c[2]*(_c[0]-_cbase[0])/_cbase[2]+c[0]);
      _local_c[1]=(T)(c[3]*(_c[1]-_cbase[1])/_cbase[3]+c[1]);


      if(_invertx) _local_c[0]=c[2]-_local_c[0]+2*c[0];
      if(_inverty) _local_c[1]=c[3]-_local_c[1]+2*c[1];
      

      return is_point(_local_c);
    }
    
    template <class U> bool get_area(const rect<U>& _c, const rect<U>& _cbase, rect<T>& _local_c, 
				     bool _invertx=false, bool _inverty=false){
      
      for(register int k=0;k<2;k++){
	_local_c[k+2]=(T)(_c[k+2]*1.0/_cbase[k+2]*c[k+2]);
	//	_local_c[k]=(T)(((_c[k]-_cbase[k])*1.0/_cbase[k+2])*c[k+2]);
	//	_local_c[0]=(T)(  ((_c[0]-_cbase[0])*1.0/_cbase[2])  *c[2]);
	_local_c[k]=(T)(c[k+2]*(_c[k]-_cbase[k])/_cbase[k+2]+c[k]);
      }
      

      if(_invertx) _local_c[0]=c[2]-_local_c[0]+2*c[0];
      if(_inverty) _local_c[1]=c[3]-_local_c[1]+2*c[1];

      return is_point(_local_c.c);
      
      //  bool invert[2];
    }
    
  };
    
  ///n-dimensional rectangle.
  
  template <class T> class nrect{

  public:
    
    mem<T> C;
    mem<T> D;
    
      nrect(int _dim=0){
      resize(_dim);
    }
    virtual ~nrect(){}

    bool resize(int _dim){
      C.redim(_dim);
      D.redim(_dim);
      return true;
    }

    bool nnrect_isect(nrect<T> & _rect, nrect<T> & _sect){
      for(int i=0;i<C.dim;i++){
        if(_rect.C[i]<C[i] && (_rect.C[i]+_rect.D[i])>=C[i]){
  	  _sect.C[i]=C[i];
  	  _sect.D[i]= ((_rect.C[i]+_rect.D[i])<(C[i]+D[i])) ? 
	    _rect.D[i]-C[i]+_rect.C[i] : D[i];
  	}
        else
	  if(_rect.C[i]>=C[i] && _rect.C[i]<(C[i]+D[i])){
  	    _sect.C[i]=_rect.C[i];
  	    _sect.D[i]= ((_rect.C[i]+_rect.D[i])<(C[i]+D[i])) ? 
	      _rect.D[i] : (C[i]+D[i]-_rect.C[i]);
  	  }
	  else
  	  return false;
      }
      
      return true;
    }
    
    bool nis_point(T * _point){
      for(int i=0;i<C.dim;i++)
	if(_point.C[i]<C[i] || _point.C[i]>C[i]+D[i])
	  return false;
      return true;
    }
    
  };
  
}
#endif
