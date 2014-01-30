//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __COLORMAP_HH__
#define __COLORMAP_HH__

#include "vec.hh"
#include "lst.hh"


namespace qk{
  
  template <class T> 
  class colormap_value : public vec<T>{
  public:
    using vec<T>::set_all;
    using vec<T>::c;
    
    colormap_value(): vec<T>(5){
      set_all(0);
    }
    virtual ~colormap_value(){}
    T& get_color(const unsigned char col){return c[col]; }
    T& get_value(){return c[4]; }
    
  };
  
  template <class T>
  class colormap : public lst<colormap_value<T>*>{
    
  public:
    
    colormap(){}
    virtual ~colormap(){}

    using lst<colormap_value<T>*>::ttd;
    using lst<colormap_value<T>*>::add;
    using lst<colormap_value<T>*>::dim;
    using lst<colormap_value<T>*>::G;
    using lst<colormap_value<T>*>::D;
    
    template <typename VT>
    void get_color(const VT& _value, vec<T>&color){
      T value=(T)_value;
      
      if(dim==0) throw exception("No color in colormap");


      if(dim==1){
	color=*(**G);
	return;
      }
      
      if(value <= (**G)->get_value() ){
	color=*(**G);
	return;
      }
      
      
      if(value >= (**D)->get_value() ){
	color=*(**D);
	return;
      }
      
      
      lel<colormap_value<T>*> *lcv=G;

      while(lcv && (**lcv)->get_value()<value) lcv=lcv->d;
      if(!lcv || lcv==G){
	throw exception("Bug illogic here");
      }
      T* v[2]={ (**(lcv->g))->c ,(**lcv)->c}; 
      
      T d=v[1][4]-v[0][4];
      T f[2]={value-v[0][4],v[1][4]-value };
      
      for(int c=0;c<2;c++){f[c]=1.0- f[c]/d; }
      //MINFO << "ff: "<<f[0] << ", " << f[1] << endl;
      
      for(int c=0;c<4;c++){
	color[c]=f[0] *v[0][c]+f[1] *v[1][c] ;
      }
    }
    
  };

  template <class T>
  class colormap_array : public mem<colormap_value<T> >{
    
  public:
    
    colormap_array(){}
    virtual ~colormap_array(){}

    using mem<colormap_value<T> >::c;
    using mem<colormap_value<T> >::dim;
    using mem<colormap_value<T> >::redim;
  
    colormap_array& operator=(const colormap<T>& cmap){
 
      redim(cmap.dim);
      lel<colormap_value<T>*> *lcv=cmap.G;
      for(int ci=0;ci<dim;lcv=lcv->d,ci++)
	c[ci]=*(**lcv);
       return *this;
    }
  
    template <typename VT>
    void get_color(const VT& _value, vec<T>&color){
      T value=(T)_value;
      
      if(dim==0) throw exception("No color in colormap!");

      if(dim==1){
	color=c[0];
	return;
      }
      

      

      if(value <= c[0].get_value() ){
	color=c[0];
	return;
      }
      
      
      if(value >= c[dim-1].get_value() ){
	color=c[dim-1];
	return;
      }
      
      
      int ci=0;

      while(ci<dim && c[ci].get_value()<value){ ci++;
	//	cout << "ci="<<ci<< " V= " << c[ci].get_value() << " VIN="<<value << endl;
      }
     
      if(ci>=dim || ci==0){
	cout << "c0="<<c[0].get_value() << " clast= " << c[dim-1].get_value() << " VIN="<<value << endl;
	throw exception("Bug illogic here");
      }
      
      T* v[2]={ c[ci-1].c ,c[ci].c}; 
      
      float d=v[1][4]-v[0][4];
      float f[2]={ (float)(value-v[0][4]), (float)(v[1][4]-value) };
      
      for(int c=0;c<2;c++){f[c]=1.0- f[c]/d; }
      //MINFO << "ff: "<<f[0] << ", " << f[1] << endl;
      
      for(int c=0;c<4;c++){
	color[c]=(T) (f[0] *v[0][c]+f[1] *v[1][c]) ;
      }
    }
    
  };
  

  
  
}




#endif
