//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __VEC_HH__
#define __VEC_HH__

#include <iomanip>
#include <math.h>
#include <sstream>

#include <qk/mem.hh>

namespace qk{


#ifndef MIN
#define MIN(A,B) ((A<B) ? A : B)
#endif

#ifndef MAX
#define MAX(A,B) ((A>B) ? A : B)
#endif 

  //  template <class T> class mat;
  

  /**                                                                                                                                                    
   *  @brief Vector of numerical type objects.                                                                                     
   *                                                                                                                                                     
   *  The vector template is a subclass of an array \refer dmem of elements. The template
   *  base type must implement arithmetic operators (+,-,*,/) as well as comparison (= and !=).
   *  It is immediately the case with standard numerical types (int, float, double, etc..)
   */ 

  
  template <class T> class vec
    : public mem<T>{
    
  public:
    
    using mem<T>::c;
    using mem<T>::dim;
    using mem<T>::redim;
    using mem<T>::check_dim;
    
    
    vec(int _dim=0):mem<T>(_dim){}
    vec(const vec &_c):mem<T>(_c){}  

    
    T sum() const{
      T s=0;
      for (register int i=0;i<dim;i++)
	s+=c[i];
      return s;
    }
    /**                                                                                                                                                    
     *  @brief  Addition operator.                                                                                     
     *  @param _v1 The vector to add
     *  @return  New sum vector object.
     *                                                                                                                                                     
     *  Addition of vectors. 
     */


    vec operator + (const vec & v1) const{
      if(dim!=v1.dim) throw exception("Invalid dimension");
      vec v(dim);

      for (register int i=0;i<dim;i++)
	v[i]=c[i]+v1[i];
      return v;
    }
    vec operator - (const vec & v1) const{
      if(dim!=v1.dim) throw exception("Invalid dimension");
      vec v(dim);

      for (register int i=0;i<dim;i++)
	v[i]=c[i]-v1[i];
      return v;
    }
    T operator * (const vec & _v) const{
      if(dim!=_v.dim) throw exception("Invalid dimension");
      T v=0;

      for (register int i=0;i<dim;i++)
	v+=c[i]*_v[i];
      return v;
    }
    vec operator / (const T div) const{
      vec v(dim);

      for (register int i=0;i<dim;i++)
	v[i]=c[i]/div;
      return v;
    }
    vec operator * (const T f) const{
      vec v(dim);

      for (register int i=0;i<dim;i++)
	v[i]=c[i]*f;
      return v;
    }
    
    const vec & operator += (const vec & v){
      for( register int i=0; i<dim;i++)
	c[i]+=v[i];
      return *this;
    }
    const vec & operator -= (const vec & v){
      for( register int i=0; i<dim;i++)
	c[i]-=v[i];
      return *this;
    }

    const vec & operator *= (const vec & v){
      for( register int i=0; i<dim;i++)
	c[i]*=v[i];
      return *this;
    }

    const vec & operator /= (const vec & v){
      for( register int i=0; i<dim;i++)
	c[i]/=v[i];
      return *this;
    }

    vec& operator /= (const T & _o){
      for(register int i=0;i<dim;i++)
	c[i]/=_o;  
      return *this;
    }
    
    
    vec& operator *= (const T & _o){
      for(register int i=0;i<dim;i++)
	c[i]*=_o;  
      return *this;
    }
    
    vec& operator += (const T & _o){
      for(register int i=0;i<dim;i++)
	c[i]+=_o;  
      return *this;
    }


    vec& operator -= (const T & _o){
      for(register int i=0;i<dim;i++)
	c[i]-=_o;  
      return *this;
    }

    virtual ~vec(){}    

    /**                                                                                                                                                    
     *  @brief  Initialises all elements to same value.                                                                                     
     *  @param _val The value to set.
     *                                                                                                                                                     
     *  
     */

    inline void set_all(T _val){
      for(register int i=0;i<dim;i++) c[i]=_val;      
    }


    /**                                                                                                                                                    
     *  @brief Return the minimum value of vector.                                                                                     
     *  @param _id Optional. Pointer to the array id where the max value was found.
     *                                                                                                                                                     
     *  @Return The minimum value found in the array.
     */
    
    T min(int * _id=0x0) const{
      
 
      if(dim==0){
	MWARN << "Null array size" << endl;
	throw exception("Null array size");
      } 

      T o; 
      o=c[0]; 

      if(_id!=NULL){
        *_id=0;
        for (register int i = 0; i<dim; i++) 
	  if (c[i] < o) { o = c[i]; *_id=i;}
      }
      else
	for (register int i = 0; i<dim; i++) if (c[i] < o)  o = c[i];      
      return o;
    }

    /**                                                                                                                                                    
     *  @brief Return the maximum value of vector.                                                                                     
     *  @param _id Optional. Pointer to the array id where the max value was found.
     *                                                                                                                                                     
     *  @Return The maximum value found in the array.
     */

    T max(int * _id=0x0) const{
      
 
      if(dim==0){
	MWARN << "Null array size" << endl;
	throw exception("Invalid dimension");

      } 

      T o; 
      o=c[0]; 
      
      if(_id!=NULL){
        *_id=0;
        for (register int i = 0; i<dim; i++) 
	  if (c[i] > o)  { o = c[i]; *_id=i;}
      }
      else
	for (register int i = 0; i<dim; i++) 
	  if (c[i] > o)  o = c[i];
      
      return o;
    }

    /**                                                                                                                                                    
     *  @brief Return the median value.                                                                                     
     *  @param _in_place Optional. Specifies if the vector should be 
     *  sorted in place, then modifying data structure. If false (default),
     *  then a copy of the data is created for data sorting.
     *                                                        
     *  @param _position Optional. Specifies at which fraction of the sorted buffer 
     *  to pickup the value. Default is 0.5 (the actual median).
     *  
     *  @Return The median value of the vector.
     */

    T med(bool _in_place=false, double _position=.5){
      vec<T> * cp; T m;
      if(!_in_place)
	{  cp=new vec<T>; *cp=*this;}
      else cp=this;
      
      cp->sort();
      m=cp->c[(int)(_position*cp->dim)];
      
      if(!_in_place) delete cp;
      return m;
    }

    T med_mean(double* _pos_bnds, bool _in_place=false){
      
      vec<T> * cp; T m=0;

      if(!_in_place)
	{  cp=new vec<T>; *cp=*this;}
      else cp=this;
      
      cp->sort();
      register int i,is=(int)(_pos_bnds[0]*cp->dim),ie=(int)(_pos_bnds[1]*cp->dim);
      
      if(is==ie) m=cp->c[is];
      else{
	for(i=is;i<ie;i++)
	  m+=cp->c[i];
	m/=(ie-is);
      }
      
      if(!_in_place) delete cp;
      return m;
    }

    inline double mean(){
      register double m=0; 
      for(register int i=0;i<dim;i++) m+=c[i]; 
      return m*1.0/dim;
    }
    
    template <typename MT> 
    double mean(mem<MT>& _mask){
      if(_mask.dim!=dim) throw exception("Invalid dimension");
      register double m=0; int np=0;
      for(register int i=0;i<dim;i++
	  )if(!_mask[i]){
	  m+=c[i]; 
	  np++;
	}
      return m*1.0/np;
    }
    
    double var(double * _mean=0x0){
      register double v=0, m;
      register T x;

      if(_mean==NULL)
	m=mean(); 
      else
	m=*_mean;
      
      for(register int i=0;i<dim;i++){
	x=c[i]; v+=x*x; 
      }
      v/=dim;
      v-=m*m;
      return v;
    }
    
    template <typename MT> 
      double var(mem<MT>& _mask, double * _mean=0x0){
      if(_mask.dim!=dim) throw exception("Invalid dimension");
      register double v=0, m;
      register T x;
      int np=0;
      if(_mean==NULL)
	m=mean(_mask); 
      else
	m=*_mean;
      
      for(register int i=0;i<dim;i++){
	if(!_mask[i]){
	  x=c[i]; v+=x*x;
	  np++;
	} 
      }
      v/=np;
      v-=m*m;
      return v;
    }
    inline double sigma(double *_mean=0){
      return sqrt(var(_mean));      
    }

    template <typename MT> 
      inline double sigma(mem<MT>& _mask, double *_mean=0){
      return sqrt(var(_mask,_mean));      
    }
    
    void create_histo_n(vec<T>& _bin_positions, vec<int>& _bin_values, int _nbins, T* _bin_range=NULL){

      T range[2];

      if(!_bin_range){
	range[0]=min();
	range[1]=max();
      }else{
	range[0]=_bin_range[0];
	range[1]=_bin_range[1];
      }

      T bin_size = (range[1]-range[0])/_nbins;

      _bin_positions.redim(_nbins);
      _bin_values.redim(_nbins);
      _bin_values.set_all(0);

      for(register int j=0;j<_nbins;j++)
	_bin_positions[j]=range[0]+(j+.5)*bin_size;
      
      for(register int i=0;i<dim;i++)
	for(register int j=0;j<_nbins;j++)
	  if(c[i]>=range[0]+j*bin_size && c[i]<range[0]+(j+1)*bin_size ){
	    _bin_values[j]++;
	    break;
	  }      
    }

    void create_histo_size(vec<T>& _bin_positions, vec<int>& _bin_values, T& _bin_size, T* _bin_range=NULL){
      int nbins;
      T range[2];

      if(_bin_range){
	range[0]=_bin_range[0];
	range[1]=_bin_range[1];
      }else{
	range[0]=min();
	range[1]=max();
      }

      nbins=(_bin_range[1]-_bin_range[0])/_bin_size;
      
      if(nbins<1){
	MERROR << "Invalid arguments "<<endl;
	exception("Invalid arguments");
      }

      create_histo_n(_bin_positions,_bin_values,nbins,range);
      
    }

    /*
  void create_histo(dvec & points, dvec & _bins, char * _name, char * _out)
  {
  
    int i,j;
    
    double min = points.min();
    double max = points.max();
  
    double range = max-min;
    double step = .005;
  
    
    int nbin = (int) (range/step + 1);
    printf("Creating histo: min %g, max %g, range %g step %g, nbin = %d\n",min,max,range,step, nbin);
  
  
    _bins.redim(nbin);
    _bins.set_all(0);
  
    
    for(j=0;j<points.dim;j++)
      {
        //      printf("point %d %g\n",j,points[j]);
        for(i=0;i<nbin;i++)
  	{
  	  if(points[j] >=(min+step*i))
  	    if (points[j] <=(min+step*(i+1)))
  	      {
  		//printf("OK\n");
  		_bins[i]+=1;
  	      }
  	}
      }
  
    if(_name!=NULL)
      {
        istr fout;
        FILE * f;
  
        fout.set_str(_name);
        fout+=".txt";
        
        f=fopen(fout.c_str(), "w+");
        for (i=0;i<nbin;i++) fprintf(f,"%g %g\n", min + (i+.5)*step, _bins[i]);
        fclose(f);
        
        if(_out!=NULL)
  	{
  	  fout.set_str(_name);
  	  fout+=".gnu";
  	  
  	  FILE * gnupl = fopen(fout.c_str(),"w+");
  	  
  	  fprintf(gnupl,"set terminal png x000000 xffffff\n");
  	  fprintf(gnupl,"set xlabel \"Redshift\"\n");
  	  fprintf(gnupl,"set grid\n");
  	  fprintf(gnupl,"set ylabel \"Number of objects\"\n");
  	  fprintf(gnupl,"set title \"%s (bin=%g)\"\n", _name, step);
  	  
  	  fprintf(gnupl,"set out \"%s/%s_histo.png\"\n",_out, _name);
  	  fprintf(gnupl,"plot \"%s.txt\" notitle w hist\n" ,_name);
  	  
  	  fprintf(gnupl,"set terminal post\n");
  	  
  	  fprintf(gnupl,"set out \"%s/%s_histo.ps\"\n",_out, _name);
  	  fprintf(gnupl,"plot \"%s.txt\" notitle w hist\n" ,_name);
  	  
  	  
  	  fclose(gnupl);
  
  	  char nn[2048];	  
  	  sprintf(nn,"gnuplot %s",fout.c_str());
  	  system(nn);
  	}
      }
  
  }
    */

    double covariance(vec<double>& _b) {
      if(!check_dim(_b)){
	MERROR << "Incompatible dimensions."<<endl;
	throw exception("Invalid dimension");
      }
      double cov = 0, m1 = mean(), m2 = _b.mean();
      for(register int i=0;i<dim;i++)
	cov += (c[i]-m1)*(_b.c[i]-m2);
      cov /=dim;
      return cov;
    }
    
    double correlation(vec<T>& _b) {
      double s1 = sigma(), s2 = _b.sigma();
      return covariance(_b)/s1/s2;
    }
    
    double cross_correlation(vec<T>& _b, int _n) {
      double cc=0;
      register int i;
      for(i=0;i<dim-_n;i++)
	cc+=c[i]*_b[i+_n];

      //cyclic rotation of buffer. ok?
      for(i=dim-_n;i<dim;i++)
	cc+=c[i]*_b[i-dim+_n];

      return cc;
      
    }

    void seq(T _start, T _step){
      for(int i=0;i<dim;i++)
	c[i]=_start+i*_step;
    }
    
    void write_ascii(stream& _s){
      ostringstream oss;
      register int i;
      for(i=0;i<dim;i++)
	oss << std::setprecision(10) << c[i] << endl; 

      _s.io((void*)oss.str().c_str(),oss.str().size());
      
    }


    void read_ascii(stream& _s){
      string sline;
      int ne=0;

      while(1){
	
	try{
	  _s.read_line(&sline);
	}catch (exception& e){
	  break;
	}

	ne++;
	sline.clear();
      }
      _s.rewind();
      redim(ne); ne=0;
      while(1){
	
	try{
	  _s.read_line(&sline);
	}catch (exception& e){
	  break;
	}
	
	
	istringstream oss;
	oss.str(sline);
	oss >> std::setprecision(10) >>c[ne];
	ne++;
	sline.clear();
      }
    }


    int zeros(mem<T> & _zpos){
     
      if(dim==0) return 0;
      int sig= (c[0]>0) ? 1:0,sige;
      mem<T> zt(dim);
      int nz=0;
      
      int i;double a,b;
      for(i=1;i<dim;i++){
	sige=(c[i]>0) ? 1:0;
        if(sige!=sig){
	  a=(c[i]-c[i-1]);
	  b=c[i]-a*i;
	  zt[nz]=-b/a;
	  sig=sige;
	  nz++;
	}
      }
      
      _zpos.redim(nz);
      for(i=0;i<nz;i++) _zpos[i]=zt[i];
      return nz;
    }
    


    void get_var_bloc(mem<T> & _var_bloc, double * _mean=0x0){
      _var_bloc.redim(dim);
      double m;
      
      if(_mean==NULL)m=mean();else m=*_mean;
      
      for(register int i=0;i<dim;i++) _var_bloc[i]=(T)sqrt((c[i]-m)*(c[i]-m));
    }
    
    void partition(vec<int> & _part, T* _cuts){
      register int j,pow;
      
      T d=_cuts[1]-_cuts[0];
      
      for(j=0;j<_part.dim;j++) _part[j]=0;
      
      for(j=0;j<dim;j++){   
	if(c[j]>=_cuts[0] && c[j]<=_cuts[1]){
  	  pow=(int)((c[j]-_cuts[0])*1.0/d*(_part.dim-1));
  	  _part[pow]++;
  	}
      }
    }
    void get_bounded_zone(T * _bounds, int * _zone, int _istart=0, int _dir=1){
      register int i;
      
      if(c[_istart]>=_bounds[0] && c[_istart]<=_bounds[1]) _zone[0]=_istart; 
      else { _zone[0]=-1; _zone[1]=-1; return; }
      
      if(_dir)
	for(i=_istart;i<dim;i++)if(c[i]>=_bounds[0] && c[i]<=_bounds[1]) _zone[1]=i; else break;
      else
	for(i=_istart;i>=0;i--)if(c[i]>=_bounds[0] && c[i]<=_bounds[1]) _zone[1]=i; else break;
    }

    int get_next_extremum(int _istart, int & _l, int & _r, int& _type=1){
      int i=_istart;
      int e=0;
      _l=-1;
      _r=-1;
      
      if(_type){
        while(i<dim-2){
  	  if( ((c[i+2]-c[i+1]) > 0) != ((c[i+1]-c[i]) >0)){
	    if((c[i+2]-c[i+1])<0) {
	      if(_l==-1){
		_l=_istart;
	      }
	      e=i+1;
	    }
	    else{
	      if(_l==-1) {
		_l=i;
	      }
	      else
		if(e!=-1){	  
		  _r=i+1;
		  return e;
		}
	    }
	    
	  }
  	  
  	  i++;
  	  
  	}
        
      }
      else
	{
	  
	}
      
      e=-1;
      
      return e;
    }

    T get_square_norm() const{
      T n=0;
      for(int i=0;i<dim;i++)
	n+=c[i]*c[i];
      return n;
    }

    T norm(){
      T n=0;
      for(int i=0;i<dim;i++)
	n+=c[i]*c[i];
      n=sqrt(n);
      for(int i=0;i<dim;i++)
	c[i]/=n;
      return n;
    }

    void quicksort(mem<int> * _ids=0x0){
      quicksort(0,dim-1,_ids);
    }
    

    
    /** 
     * @brief Swap two variables
     * 
     * @param _a 
     * @param _b 
     * @param _ids 
     */

    inline void swap(int _a, int _b,mem<int> * _ids=0x0){
      T temp;
      temp = c[_a];
      c[_a] = c[_b];
      c[_b] = temp;
      
      if(_ids){
	int idtmp=(*_ids)[_a];
	(*_ids)[_a]=(*_ids)[_b];
	(*_ids)[_b]=idtmp;
      }
    }


    /** 
     * @brief Find the index of the Median of the elements
     * of array that occur at every "shift" positions.
     * 
     * @param left 
     * @param right 
     * @param shift 
     * @param _ids 
     * 
     * @return 
     */

    int findMedianIndex(int left, int right, int shift, mem<int> * _ids=0x0){
      int i, groups = (right - left)/shift + 1, k = left + groups/2*shift;
      for(i = left; i <= k; i+= shift){
	int minIndex = i, j;
	T minValue = c[minIndex];
	for(j = i; j <= right; j+=shift)
	  if(c[j] < minValue){
	    minIndex = j;
	    minValue = c[minIndex];
	  }
	swap(i, minIndex,_ids);
      }
      return k;
    }
    
    /** 
    * @brief Computes the median of each group of 5 elements and stores
    * it as the first element of the group. Recursively does this
    * till there is only one group and hence only one Median.
     * 
     * 
     * @param left 
     * @param right 
     * @param _ids 
     * 
     * @return 
     */
    
    T findMedianOfMedians(int left, int right, mem<int> * _ids=0x0){
      if(left == right)
	return c[left];
      
      int i, shift = 1;
      while(shift <= (right - left)){
        for(i = left; i <= right; i+=shift*5)
	  {
            int endIndex = (i + shift*5 - 1 < right) ? i + shift*5 - 1 : right;
            int medianIndex = findMedianIndex(i, endIndex, shift,_ids);
	    
            swap(i, medianIndex,_ids);
	  }
        shift *= 5;
      }
      
      return c[left];
    }


    /**
     * @brief Partition the array into two halves and return the
     *        index about which the array is partitioned.
     */
    
    
    
    int partition(int left, int right, mem<int> * _ids=0x0){
      /**
	 Makes the leftmost element a good pivot,
	 specifically the median of medians
      */
      findMedianOfMedians(left, right,_ids);
      int pivotIndex = left, index = left, i;
      T pivotValue = c[pivotIndex];
      
      swap(pivotIndex,right,_ids);

      for(i = left; i < right; i++){
        if(c[i] < pivotValue){
	  swap(i, index, _ids);
	  index += 1;
	}
      }
      swap(right, index,_ids);
      
      return index;
    }
    

    
    /** 
     * @brief Quicksort the array.
     */
    
    void quicksort(int left, int right, mem<int> * _ids=0x0){
      if(left >= right)
	return;
      
      int index = partition(left, right,_ids);
      quicksort(left, index - 1,_ids);
      quicksort(index + 1, right,_ids);
    }
    
    
    inline void sort(mem<int> * _ids=0x0, T* _bounds=0){
      quicksort(_ids);
      //bubblesort(_ids);
    }

    void bubblesort(mem<int> * _ids=0x0, T* _bounds=0){
      int nper=0;
      T tmp;  
      
      if(_ids!=NULL){
	int idtmp;
	
	for(int i=0;i<dim-1;i++)
	  if(c[i+1]<c[i]){
	    idtmp=(*_ids)[i];
	    (*_ids)[i]=(*_ids)[i+1];
	    (*_ids)[i+1]=idtmp;
	    tmp=c[i];
	    c[i]=c[i+1];
	    c[i+1]=tmp;
	    nper++;
	  }
      }else
	for(int i=0;i<dim-1;i++)
	  if(c[i+1]<c[i]){
	    //	    MINFO << "nper= "<<nper<<" swap "<< c[i] << ", " << c[i+1] << endl;
	    tmp=c[i];
	    c[i]=c[i+1];
	    c[i+1]=tmp;
	    nper++;
	  }
      
      if (nper!=0){
	//	MINFO << "nper="<<nper<<endl;
	bubblesort(_ids);
      }
    }
   
    vec operator ^ (const vec<T> & v){
      vec<T> s (3);
    
      s[0] = c[1] * v[2] - c[2] * v[1];
      s[1] = c[2] * v[0] - c[0] * v[2];
      s[2] = c[0] * v[1] - c[1] * v[0];
      
      return s;
    }

    void apodisation(double _cut){
      int i;
      double lcut = _cut*dim;
      for(i=0;i<(int)lcut;i++) c[i]=c[i]*0.5 * (cos(M_PI * (1 - i*1.0/lcut)) + 1); 
      for(i=dim-1;i>(int)(dim-1-lcut);i--) c[i] =c[i]*0.5 * (cos(M_PI * (1 - i*1.0/lcut)) + 1); 
    }

  };
  
  
  
}
#endif
