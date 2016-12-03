//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __MAT_HH__
#define __MAT_HH__

#include "vec.hh"
#include "rect.hh"

namespace qk{

  ///A matrix of objects of type T. The type T must implement arithmetic,
  ///equality and comparison operators.

  template <typename T> class mat 
    : public vec<T>{
  public:
    using mem<T>::c;
    using mem<T>::dim;
    using vec<T>::set_all;
    using vec<T>::min;
    using vec<T>::max;
    using vec<T>::mean;
    using vec<T>::med;
    using vec<T>::sigma;
    
    //    using vec<T>::class_id;

    ///Constructor.
    ///\param _d0 : The row dimension (number of columns) of the matrix. 
    ///\param _d1 : The column dimension (number of rows) of the matrix. 
    
    mat (int _d0=0, int _d1=0)  
      :vec<T>(0){
      redim(_d0,_d1);
    }    
    
    ///Constructor.
    
    mat (int* _dims)  
      :vec<T>(0){
      redim(_dims[0],_dims[1]);
    }    

    ///Copy constructor.

    mat (const mat & m):vec<T>(){
      operator = (m);
    }   



    ///Resizes the image.
    ///\param _d0: the new row dimension.
    ///\param _d1: the new column  dimension.

    virtual void redim(int d0, int d1){
      vec<T>::redim(d0*d1);
      dims[0] = d0;
      dims[1] = d1;
    }

    ///Resizes the image.
    ///\param _dims: A pointer to a (at least 2) sized array containing the new dimensions.
    void mredim(int * _dims) {redim(_dims[0], _dims[1]); }

    void mmap(T* _data, int _d0, int _d1){
      vec<T>::mmap(_data,_d0*_d1);
      dims[0]=_d0;
      dims[1]=_d1;
    }
    
    ///Destructor.
    
    virtual ~mat (){}
    

    // virtual void register_commands(cmd_definitions& _commands);
    // virtual bool exec_command(cmd_def& _command);

    
    ///Equality operator. 
    ///\param _ms : The matrix to copy data from. 

    const mat<T> & operator = (const mat<T> & ms){
      //cout << "OP EQUAL!"<<endl;
      if (this != &ms){
	dims[0] = ms.dims[0];
	dims[1] = ms.dims[1];      
	vec<T>::operator=(ms);
      }    
      return *this;
    }
    
    
    ///Addition operator.
    ///\param m : The matrix to be added .


    mat<T> operator + (mat<T> & m)
    {
      mat<T> ret(*this);
      for(register int i=0;i<dim;i++)
	ret[i]+=m[i];
      
      return ret;
    }
    
    ///Difference operator.
    ///\param m : The matrix to be substracted .

    mat<T> operator - (mat<T> & m)
    {
      mat<T> ret(*this);
      for(register int i=0;i<dim;i++)
	ret[i]-=m[i];
      return ret;
    }
    
    


    ///Multiplication operator. If the dimensions doesn't suit matrix 
    ///multiplication rules, an excpetion is thrown. 

    ///\param m : The matrix to be multiplied.

    
    mat<T> operator * (mat<T> & m)
    {
      mat<T> ret;
      //condition : dims[0]=m.dims[1]
      
      ret.redim(dims[1], m.dims[0]);
      mat<T> trans;
      trans=m.transpose();
      T tmp;
      register int i,j,k;
      for(i=0;i<dims[1];i++)
        for(j=0;j<m.dims[0];j++)
	  {
	    tmp=0;
	    for(k=0;k<dims[0];k++)
	      tmp+=c[i*dims[0]+k]*trans[i*dims[0]+k];
	    ret[i*m.dims[0]+j]=tmp;
	  }
      
      return ret;
    }
    

    
    ///Multiplication operator. If the dimensions doesn't suit matrix 
    ///multiplication rules, an excpetion is thrown. 

    ///\param m : The vector to be multiplied.


    
    vec<T> operator * (const vec<T> & v)
    {
      vec<T> r(dims[1]);
      //  cout << r.c << endl;
      T tmp;
      
      register int i,j;
      for(i=0;i<dims[1];i++)
	{
	  tmp=0;
	  for(j=0;j<dims[0];j++)
	    {
	      //	  cout << "c=" << c[i*dims[0]+j] << endl;
	      tmp+=c[i*dims[0]+j]*v[j];
  	}
	  r[i]=tmp;
	}
      return r;
    }
    

  
    ///Multiplication operator by a base type T. Every matrix element
    ///will be multiplied by the value given by o.
    ///\param o : The base type object to be multiplied.

  mat<T> operator * (T & o)
  {
    mat<T> ret(*this);
    for(register int i=0;i<ret.dim;i++)
      ret[i]*=o;
    return ret;
  }
  


    ///Division operator by a base type T. Every matrix element
    ///will be divided by the value given by o.
    ///\param o : The base type object to be divided.
  
  
  
  mat<T> operator / (T & o)
  {
    mat<T> ret(*this);
    for(register int i=0;i<ret.dim;i++)
      ret[i]/=o;
    return ret;
  }

  
    ///Initializes the matrix elements to diag(1, ..., 1). 
    ///If the matrix is not square, an exception is thrown.

  
  void identity (){
    int i;
    set_all(0);
    if (dims[0] == dims[1])
      for (i = 0; i < dims[0]; i++)
        c[i * dims[0] + i] = 1;
  }
  
    ///Transposes the matrix. 
    
    mat<T> &  transpose(){
      mat<T> ret;
      ret.redim(dims[1], dims[0]);
      for(register int j=0;j<dims[1];j++)
	for(register int i=0;i<dims[0];i++)
	  ret[i*dims[1]+j]=c[j*dims[0]+i];
      operator=(ret);
      return *this;
    }
    
  
  
  void io(stream& _s){
    
    _s.io((char*)dims, sizeof(int)*2);
    vec<T>::io(_s);
  }
  
  
  void get_vector(unsigned int _type, int _id, vec<T> & _vec){
    if(_type)
      get_col(_id,_vec);
    else
      get_line(_id,_vec);
  }

  
  void get_line(int _lid, vec<T> & _line){

    if(_lid<0||_lid>=dims[1]){
      MERROR << "Out of bounds " << _lid << endl;
      throw exception("Out of bounds");
    }
    //checking and imposing compatible vector dimension.  
    _line.redim(dims[0]);

    T* cc=c+_lid*dims[0];
    for(register int i=0;i<dims[0];i++)
      _line.c[i]=*cc++;
  
  }
  
  
  void get_col(int _cid, vec<T> & _col){
  
    if(_cid<0||_cid>=dims[0]){
      MERROR << "Out of bounds " << _cid << endl;
      throw exception("Out of bounds");
    }

    //checking and imposing compatible vector dimension.
    _col.redim(dims[1]);
    
    T *cc=c+_cid;
    for(register int i=0;i<dims[1];i++){
      _col.c[i]=*cc;
      cc+=dims[0];
    }
  }


  
  void put_line(int _lid, vec<T> & _line){

    if(_lid<0||_lid>=dims[1]){
      MERROR << "Out of bounds " << _lid << endl;
      throw exception("Out of bounds");
    }
  
    if(_line.dim!=dims[0]){
        MERROR << "bad sized vector" << endl;
      throw exception("bad sized vector");
    }
    T *cc=c+_lid*dims[0];
    for(register int i=0;i<dims[0];i++)
      *cc++=_line.c[i];

    
  }
  
  
  void put_col(int _cid, vec<T> & _col){
  
    if(_cid<0||_cid>=dims[0]){
      MERROR << "Out of bounds " << _cid << endl;
      throw exception("Out of bounds");
    }

    if(_col.dim!=dims[1]){
        MERROR << "bad sized vector" << endl;
	throw exception("bad sized vector");

    }
    T *cc=c+_cid;
    for(register int i=0;i<dims[1];i++){
      *cc=_col.c[i];
      cc+=dims[0];
    }
  }

    ///Checks dimension compatibility with another matrix.

    bool check_dims(const mat<T>& _m){ return (dims[0]==_m.dims[0] && dims[1]== _m.dims[1]) ;  }

    ///Returns the value of the matrix element contained in the row _x, and column _y.
    ///\param _y : the index of the column.
    ///\param _y : the index of the row.
    ///If the specified index pair falls outside the matrix dimensions, an exception is thrown.

    inline T& value(int _y, int _x){ 
      if(!is_in(_y,_x)){
	MERROR << "Value ("<<_x<<","<<_y<<") out of bounds. dimensions :  " << dims[0] << ", " << dims[1] <<endl;
	throw exception("indexed out of bounds");
      }
      return c[_y*dims[0]+_x]; 
    }

    ///Returns the value of the matrix element contained in the row _x, and column _y. const version.
    ///\param _y : the index of the column.
    ///\param _y : the index of the row.
    ///If the specified index pair falls outside the matrix dimensions, an exception is thrown.

    inline const T& value(int _y, int _x) const { 
      if(!is_in(_y,_x)){
	MERROR << "Value ("<<_x<<","<<_y<<") out of bounds. dimensions :  " << dims[0] << ", " << dims[1] <<endl;
	throw exception("indexed out of bounds");
      }
      return c[_y*dims[0]+_x]; 
    }
    


    ///Access a matrix element specified by its row (y)
    ///and column (_x). The order (y,x) recalls that data is stored in row major order. 
    ///No boundary check is performed for the indices for performance concerns. See \ref value() for safe, checked retrieval.

    inline T & operator () (int _y, int _x){
      return c[_y*dims[0]+_x]; 
    }

    ///Access a matrix element specified by its row (y)
    ///and column (_x). The order (y,x) recalls that data is stored in row major order. 
    ///No boundary check is performed for the indices for performance concerns. See \ref value() for safe, checked retrieval.

    inline const T & operator () (int _y, int _x) const {
      return c[_y*dims[0]+_x]; 
      //      return value(_y,_x);
    }


    
    ///Returns the coordinates of the element specified by _id in the linear memory array.
    ///\param _id : The index of the element in the one dimensional array. 
    ///\param _c : A pointer to a at least 2-dimensional int array to be filled with the coordinates. 

    void get_coord(int _id, int* _c){
      _c[1]=(int)(_id*1.0/dims[0]);
      _c[0]=(int)(_id-_c[1]*dims[0]);
    }

    ///Returns the maximum value of the matrix. If _id is not null, _id will contain 
    ///the coordinates of the maximum element. 
    ///\param _id : A pointer to a at least 2-dimensional int array to be filled with the coordinates of the maximum.
    
    T max(int * _id=0x0){
      if(!_id)
	return vec<T>::max();

      int id=0;
      vec<T>::max(&id);
      get_coord(id,_id);
      return c[id];
    }

    ///Returns the minimum value of the matrix. If _id is not null, _id will contain 
    ///the coordinates of the minimum element. 
    ///\param _id : A pointer to a at least 2-dimensional int array to be filled with the coordinates of the minimum element.


    T min(int * _id=0x0){
      if(!_id)
	return vec<T>::min();

      int id=0;
      vec<T>::min(&id);
      get_coord(id,_id);
      return c[id];
    }



    mat<T> & operator +=(const mat<T> & _m){
      vec<T>::operator+=(_m);
      return *this;
    }
    mat<T> & operator -=(const mat<T> & _m){
      vec<T>::operator-=(_m);
      return *this;
    }

    mat<T> & operator /=(const mat<T> & _m){
      vec<T>::operator/=(_m);
      return *this;
    }
    mat<T> & operator *=(const mat<T> & _m){
      vec<T>::operator*=(_m);
      return *this;
    }

    mat<T> & operator +=(const T & _m){
      vec<T>::operator+=(_m);
      return *this;
    }
    mat<T> & operator -=(const T& _m){
      vec<T>::operator-=(_m);
      return *this;
    }

    mat<T> & operator /=(const T& _m){
      vec<T>::operator/=(_m);
      return *this;
    }
    mat<T> & operator *=(const T& _m){
      vec<T>::operator*=(_m);
      return *this;
    }

    ///Get the vector corresponding to a row of the matrix.
    ///Maps a matrix row data pointer to a vector c datapointer.
    ///Range of index unchecked.
    ///\param _rowid Index of row.
    ///\param _row_vector Vector to configure.

    void row(int _rowid, vec<T>& _row_vector){
      _row_vector.mmap(&c[_rowid*dims[0]],dims[0]);
    }

    
    ///Returns a pointer to the dimensions of the matrix. 
    ///\return : A pointer to the dims array containing the dimensions of the matrix.
    ///Obviously no modification of this array should be done !
    
    int * get_dims() { return dims; }

    ///Returns a dimension of the matrix.
    ///\param _did : if _did is 0, the first (row) dimension (number of columns) is returned, otherwise
    ///the column dimension is returned. 

    int get_dim(int _did) { return _did==0 ? dims[0] : dims[1]; }


    bool load_fits(const string& _file_name, int _hdu=0, int _rotate=0, int _fullimage=0);
    bool write_fits(const string& _file_name);
    
    // void get_vector(unsigned int _type, int _id, vec<T> & _vec);
    // void get_line(int _lid, vec<T> & _line);
    // void get_col(int _cid, vec<T> & _col);

    // void put_line(int _lid, vec<T> & _line);
    // void put_col(int _cid, vec<T> & _col);

    void rotate_quad(int _qid){
      mat<T> mc;
      vec<T> v;
      int i,j;
      
      switch(_qid){
      case 1:
	//mc=*this;
	//v.redim(mc.dims[0]);
	mc.redim(dims[1],dims[0]);
	
	for(i=0;i<dims[0];i++)
	  for(j=0;j<dims[1];j++)
	    mc(i,dims[1]-1-j)=(*this)(j,i);
	operator=(mc);
	/*
	for(li=0;li<mc.dims[1];li++){
	  mc.get_line(li,v);
	  put_col(li,v);
	}
	*/
	break;
      case 2:
	throw exception("Not implemented");
	break;
      case 3:
	throw exception("Not implemented");
	break;
      default:
	throw exception("Not implemented");
	break;
      };
      
    }

    void get_med_line(int _lstart, int _lend, vec<double>& _med){

      int med_size=_lend-_lstart+1,i,j;

      mem<vec<double> > meds(dims[0]);
      _med.redim(dims[0]);
      
      for(j=0;j<dims[0];j++){
	meds[j].redim(med_size);
      }
      
      for(i=0;i<med_size;i++){
	for(j=0;j<dims[0];j++){
	  meds[j][i]=operator()(_lstart+i,j);
	}
      }
      
      for(j=0;j<dims[0];j++)
	_med[j]=meds[j].med(1);
    }
    

    template <typename TypeComp> TypeComp integ_area(rect<TypeComp>* _corners, TypeComp** _fracs){

      register TypeComp a=0,f;
      register int i,j;
      
      if(_corners[0][0]!=_corners[1][0] && _corners[0][1]!=_corners[2][1]){
	for (i=_corners[0][0]+1;i<_corners[1][0];i++)
	  for (j=_corners[0][1]+1;j<_corners[2][1];j++) 
	    a+=operator()(j,i);

	for (i=_corners[0][0]+1;i<_corners[1][0];i++){
	  a+=_fracs[0][1]*operator()(_corners[0][1],i);
	  a+=_fracs[2][1]*operator()(_corners[2][1],i);
	}
	
	a+=_fracs[0][0]*_fracs[0][1]*operator()(_corners[0][1],_corners[0][0]);
	a+=_fracs[1][0]*_fracs[1][1]*operator()(_corners[1][1],_corners[1][0]);

	for (j=_corners[0][1]+1;j<_corners[2][1];j++){
	  a+=_fracs[0][0]*operator()(j,_corners[0][0]);
	  a+=_fracs[1][0]*operator()(j,_corners[1][0]);
	}
	
	a+=_fracs[2][0]*_fracs[2][1]*operator()(_corners[2][1],_corners[2][0]);
	a+=_fracs[3][0]*_fracs[3][1]*operator()(_corners[3][1],_corners[3][0]);
      }else
	if(_corners[0][0]==_corners[1][0]) {

	  if(_corners[0][1]==_corners[2][1]) 
	    a=operator()(_corners[0][1],i);
	  else{
	    f=_fracs[0][1]+_fracs[2][1]-1.0;
	    for (j=_corners[0][1]+1;j<_corners[2][1];j++)
	      a+=f*operator()(j,_corners[0][0]);

	    a+=f*_fracs[0][1]*operator()(_corners[0][1],_corners[0][0]);
	    a+=f*_fracs[1][1]*operator()(_corners[1][1],_corners[1][0]);

	  }
	}else{
	  f=_fracs[1][0]+_fracs[0][0]-1.0;
	  for (i=_corners[0][0]+1;i<_corners[1][0];i++)
	    a+=f*operator()(_corners[0][1],i);

	  a+=f*_fracs[0][0]*operator()(_corners[0][1],_corners[0][0]);
	  a+=f*_fracs[1][0]*operator()(_corners[1][1],_corners[1][0]);
	}
      
      
      //      a/= ( (_corners[1][0]-_corners[1][0]) * (_corners[2][1]-_corners[1][1]) );
      return a;
    }


    inline double average_area(const rect<double>& _area){
      register double a=0;
      return a;
    }

    template <typename TypeGrid, typename TypeComp> void sample_grid(const rect<TypeComp>& _area, mat<TypeGrid>& _grid){
      if(_grid.dim==0) return;
      TypeComp pixel_size[2];
      TypeComp pixel_area;
      pixel_size[0]=_area[2]/_grid.dims[0];
      pixel_size[1]=_area[3]/_grid.dims[1];

      pixel_area=pixel_size[0]*pixel_size[1];

      rect<TypeComp> corners[2];

      TypeComp** fracs= new TypeComp*[4];
      for(int i=0;i<4;i++) fracs[i] = new TypeComp[2];
      
      register int i,j;
      register TypeComp x,y;
      for(i=0;i<_grid.dims[1]-1;i++)
	for(j=0;j<_grid.dims[0]-1;j++){
	  x=_area[0]+i*pixel_size[0];
	  y=_area[1]+j*pixel_size[1];

	  corners[0][0]=(int)x;
	  corners[0][1]=(int)y;
	  fracs[0][0]=1-x+corners[0][0];
	  fracs[0][1]=1-y+corners[0][1];
	  
	  x=(_area[0]+(i+1)*pixel_size[0]);
	  

	  corners[1][0]=(int)x;
	  corners[1][1]=corners[0][1];
	  fracs[1][0]=x-corners[1][0];
	  fracs[1][1]=fracs[0][1];

	  y=_area[1]+(j+1)*pixel_size[1];

	  corners[2][0]=corners[1][0];
	  corners[2][1]=(int)y;
	  fracs[2][0]=fracs[1][0];
	  fracs[2][1]=y-corners[2][1];

	  x=_area[0]+i*pixel_size[0];

	  corners[3][0]=corners[0][0];
	  corners[3][1]=corners[2][1];
	  fracs[3][0]=fracs[0][0];
	  fracs[3][1]=fracs[2][1];

	  _grid(j,i)= integ_area(corners,fracs)/pixel_area;
	}

      for(int i=0;i<4;i++) delete[] fracs[i];
      delete[] fracs;
      
    }

    template <typename TypeGrid, typename TypeComp> void sample_grid_approx(const rect<TypeComp>& _area, mat<TypeGrid>& _grid){
      if(_grid.dim==0) return;
      int pixel_size[2];
      TypeComp pixel_area;
    
      _grid.set_all(0);

      pixel_size[0]=(int)(_area[2]/_grid.dims[0]);
      pixel_size[1]=(int)(_area[3]/_grid.dims[1]);

      pixel_area=pixel_size[0]*pixel_size[1];

      register int i,j,k,l;
      register int x,y;
      for(i=0;i<_grid.dims[1];i++)
	for(j=0;j<_grid.dims[0];j++){
	  
	  x=(int)(_area[0]+j*pixel_size[0]);
	  y=(int)(_area[1]+i*pixel_size[1]);
	  
	  try{
	    for(k=x;k<x+pixel_size[0];k++)
	      for(l=y;l<y+pixel_size[1];l++)
		_grid(i,j)+= operator()(l,k);

	    _grid(i,j)/= pixel_area;
	  }
	  catch (exception& _e){
	    _grid(i,j)=0;	    
	  }
	  
	}
    }


    template <typename TypeGrid, typename TypeComp> void sample_grid_nearest(const rect<TypeComp>& _area, mat<TypeGrid>& _grid){
      _grid.set_all(0);
      
      register int i,j,sx,sy;
      register TypeComp pxsz[2];
      pxsz[0]=_area[2]/_grid.dims[0];
      pxsz[1]=_area[3]/_grid.dims[1];
      
      for(j=0;j<_grid.dims[1];j++)
	for(i=0;i<_grid.dims[0];i++){
	  sx=(int)( _area[1]+(j+.5)*pxsz[1]);
	  sy=(int)( _area[0]+(i+.5)*pxsz[0]);
	  if(is_in(sx,sy))
	    _grid(j,i)= operator()(sx,sy);
	}
      
    }
    
    void extract_from(mat<T>& _source, int* _pos) {

      int lbeg,sx,sy;

      for(register int y=0;y<dims[1];y++){
	lbeg=y*dims[0];
	sy=_pos[1]+y;
	for(register int x=0;x<dims[0];x++){
	  sx=_pos[0]+x;
	  if(!_source.is_in(sy,sx))
	    c[lbeg+x]=0;
	  else
	    c[lbeg+x]=_source(sy,sx);
	}
	
      }
    }     


    void extract_from_centered(mat<T>& _source, int* _pos) {
      int lbeg,sx,sy;
      for(register int y=0;y<dims[1];y++){
	lbeg=y*dims[0];
	sy=_pos[1]+y-dims[1]/2;
	for(register int x=0;x<dims[0];x++){
	  sx=_pos[0]+x-dims[0]/2;
	  if(!_source.is_in(sy,sx))
	    c[lbeg+x]=0;
	  else
	    c[lbeg+x]=_source(sy,sx);
	}
	
      }
    }     


    void get_rect(mat<T>& _source, int * xlim, int * ylim){
      int s[2]={xlim[1]-xlim[0], ylim[1]-ylim[0]};
      
      mredim(s);

      s[0]=xlim[0];
      s[1]=ylim[0];

      return extract_from(_source,s);

    }

    ///Extracts a _size sub-matrix from the source matrix _source.
    ///\param _source : The matrix to extract data from.
    ///\param _size : The rectangular area to extract. 
    
    void get_rect(mat<T>& _source, rect<int>& _size){
      redim(_size.c[2],_size.c[3]);
      return extract_from(_source,_size.c);
    }

    ///Returns true if pixel row y, column x falls inside image.

    inline bool is_in(int _y, int _x) const{
      if(_y<0||_y>=dims[1] || _x<0||_x>=dims[0])
	return false;
      return true;
    }
    
    ///Returns true if rectangle is included in image frame.
    
    template <typename IT>
    inline bool is_in(rect<IT>& r) const{
      for(int i=0;i<2;i++)
	if(r[i]<0 || r[i+2]<=0 || r[i+2]+r[i]>dims[i])
	  return false;
      return true;
    }
    

    ///Returns true if pixel (x,y) given by _p falls inside image.
    ///with eventual margins around the point given by _margins if
    ///not null.

    inline bool is_in(int * _p, int* _margins=0){
      if(_margins){
	if(_p[0]-_margins[0]>=0&&_p[0]+_margins[0]<dims[0]
	   &&_p[1]-_margins[1]>=0&&_p[1]+_margins[1]<dims[1])
	  return true;
      }else
	if(_p[0]>=0&&_p[0]<dims[0]&&_p[1]>=0&&_p[1]<dims[1])
	  return true;
      return false;      
    }

    void set_swap(mat<T> & _o){
      
      int k,l;
      
      redim(_o.dims[1], _o.dims[0]);
      
      for(l=0;l<_o.dims[1];l++)
	for(k=0;k<_o.dims[0];k++)
	  operator()(k,l)=_o(l,k);
      
    }


    void write_ascii(stream& _s, int _transpose=0){
      ostringstream oss;
      register int i,k;

      for(i=0;i<dims[1-_transpose];i++){
	for(k=0;k<dims[_transpose];k++)
	  oss << std::setprecision(10) << (_transpose? operator()(k,i) : operator()(i,k) )<< " ";
	oss <<  endl; 
      }
      
      _s.io((void*)oss.str().c_str(),oss.str().size());
      
    }

    void read_ascii(stream& _s, int _transpose=0){

      string ls,st;
      int g,h,ncol=0,nrow=0,ncolt;
      int lss;       
      
      while(1){

	try{
	  _s.read_line(&ls);
	}catch (exception& e){
	  break;
	}
	//	MINFO << "read line ["<< ls<<"]"<<endl;

	lss=ls.size();
	g=0;
	ncolt=0;
	while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	while(g<lss){
	  h=g;
	  while(h<lss && (ls[h]!=' '&& ls[h]!='\t')) h++;
	  
	  //	  ss=ls.substr(g,g-h);
	  ncolt++;
	  g=h;
	  while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	}
	if(!ncol) ncol=ncolt;

	if(ncolt){
	  if(ncolt!=ncol){
	    MERROR << "Bad ascii data "<< ncol << " != " << ncolt << " at line "<< nrow<< endl;
	    throw exception("Bad ascii data");
	  }
	  nrow++;
	}

	ls="";

      }
      
      if(_transpose) redim(nrow,ncol); else redim(ncol,nrow);
      
      _s.rewind();
      

      nrow=0;
      //      set_all(0);
      while(1){
	

	try{
	  _s.read_line(&ls);
	}catch (exception& e){
	  break;
	}


	g=0;
	ncolt=0;
	lss=ls.size();

	while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	while(g<lss){
	  h=g;
	  while(h<lss && (ls[h]!=' '&& ls[h]!='\t')) h++;
	  
	  //	  ss=ls.substr(g,g-h);
	  ncolt++;
	  g=h;
	  while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	}

	if(ncolt){
	  g=0;
	  ncolt=0;
	  
	  while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	  while(g<lss){
	    h=g;
	    while(h<lss && (ls[h]!=' '&& ls[h]!='\t')) h++;
	    
	    if(_transpose) operator()(ncolt,nrow)=0; else operator()(nrow,ncolt)=0;
	    st=ls.substr(g,h-g) ;
	    istringstream oss;
	    oss.str(st);
	    oss >> std::setprecision(10) >>  (_transpose? operator()(ncolt,nrow) : operator()(nrow,ncolt));
	    ncolt++;
	    g=h;
	    while(g<lss && (ls[g]==' '|| ls[g]=='\t')) g++;
	  }
	  nrow++;
	}
	ls="";
      }
      
    }
    
    
    ///The dimensions of the matrix. 
    ///
    ///This data member is made public for
    ///practical usage reasons, but must be considered read-only !

    int dims[2];
    
  };
  

}
#endif
