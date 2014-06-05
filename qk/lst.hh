//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __PILE_H__
#define __PILE_H__

#include <iostream>
#include <typeinfo>

#include "cnt.hh"
#include "ansi.h"
#include "exception.hh"

///Usefull macro to loop lists.
#define Loop(List,Iter) for(Iter=List.G;Iter!=NULL;Iter=Iter->d)
///Usefull macro to loop lists (pointer version).
#define Loopp(List,Iter) for(Iter=List->G;Iter!=NULL;Iter=Iter->d)
///Usefull macro to loop lists (When inside a list class itself). Sorry DaViDe for CaMelCase !
#define LoopMe(Iter) for(Iter=G;Iter!=NULL;Iter=Iter->d)

#define Loopv(List,Iter) for(Iter=List.G();Iter!=NULL;Iter=Iter->d)

namespace qk{

  ///Iterator class used in list.
  ///'lel' stands for 'list element'. It contains an arbitrary type (T moi) instance of an object, usually a pointer, containing the information of the list item. It also contains two pointers off its own type : lel<T>* g and lel<T>* d (left and right) pointing to left and right neighbours.
  ///
  ///If g/d is null, then the list element is the last/first element of a list. 
  ///
  
  template < class T > class lel{
  public:

    ///Default ctor.
    lel():g(NULL),d(NULL){}
    ///Copy ctor. Creates a copy from another iterator.
    lel(const lel<T>& lui):g(lui.g),d(lui.g),moi(lui.moi){}
    ///Copy ctor. Creates a copy from another iterator.
    lel(lel<T>& lui):g(lui.g),d(lui.g),moi(lui.moi){}

    lel(const T& lui):g(NULL),d(NULL){moi=lui;}
    ///Dtor. Does nothing.
    ~lel(){}

    ///Getter for object, but useless in 'all public' class model, use 'moi' directly.

    inline T& get_object() const {return moi;}

    ///Setter for object, but useless in 'all public' class model.

    inline void set_obj(T& _T) { moi=_T;}

    ///Setter for object, but useless in 'all public' class model.

    inline void set_obj(const T& _T) { moi=_T;}

    ///Copy operator.
    
    const lel<T>& operator=(const lel<T>& _lc){
      moi=_lc.moi;
      g=_lc.g;
      d=_lc.d;
      return *this;
    }
    

    ///Redefined dereference operator. 
    ///It returns a reference to the owned object.

    inline T& operator*(){
      return moi;
    }

    /*
    template <class ST>
    ST& operator()(ST& ptr){ 
      ptr=dynamic_cast<ST>(moi); 
      if(!ptr) {
	MERROR << "cast to "<< typeid(ST).name() << " failed !"<< std::endl; 
	throw exception("Invalid cast");
      } 
      return ptr; 
    }
    */

    ///Returns the right neighbour iterator, the element on the 'droite'.
    inline lel<T>* next() const { return d; }    
    ///Returns the left neighbour iterator, the element on the 'gauche'.
    inline lel<T>* prev() const { return g; }

    ///Left neighbour iterator pointer.
    lel<T>* g;
    ///Right neighbour iterator pointer.
    lel<T>* d;

    ///The contained object. 
    ///(moi means me in english).
    T moi;
  };

  


  ///A simple list of objects of type T. 

  template < class T > class olst {
    //    : public cnt<T>{
    
  public:  
    int dim;
    //using cnt<T>::dim;
    
    ///Pointers to left and right iterators. (In french L/R is Gauche/Droite, sorry, programming needs 
    ///some spice sometimes don't to be too boring, no?)
    lel<T> *G,  *D;    
    ///Constructor.
    olst():dim(0),G (NULL), D (NULL){}
    ///Copy constructor.
    olst(olst & _p) { this->operator=(_p); }
    ///Destructor.
    virtual ~olst(){
      tt();
    }

    ///Returns a reference to the object at 
    ///position \par{_id} from the beginning of the container. 
    virtual T& operator [] (int _id){
      if (_id >= dim||_id<0)
	throw exception("index out of range"); 
   
      lel<T>* index = G;    

      for (register int k = 0; k < _id; k++) index = index->d;
      if(index==NULL)
	throw exception("null iterator !");

      return index->moi;
    }

    ///Adds an element at the right/end of the container.
    ///Returns a reference to the new object.
    virtual T& add(const T& _o){
      lel<T> *nouvel = new lel < T > (_o);  
      nouvel->g = D;
  
      if (D != NULL)
	D->d = nouvel;
      else
	G = nouvel;
  
      D = nouvel;
      dim++;
      return nouvel->moi;
    }

    ///Adds an element at the beginning/left of the container.
    ///Returns a reference to the new object.
    virtual T& adg(const T& _o)  {
      lel<T> *nouvel = new lel < T > (_o);
      
      nouvel->d = G;
      
      if (G != NULL)
	G->g = nouvel;
      else
	D = nouvel;
      
      G = nouvel;
      dim++;
      return nouvel->moi;
    }

    ///Adds an element at the right/end of the container and return a pointer to the iterator.
    lel<T> * ajoute(T& nouv)  {
      lel<T> *nouvel = new lel < T > (nouv);      
      nouvel->g = D;      
      if (D != NULL)
	D->d = nouvel;
      else
	G = nouvel;      
      D = nouvel;
      dim++;
      //    cobj = nouv;      
      return nouvel;
    }

    ///Returns a pointer to the iterator at
    ///position _idx from the beginning of the container. 
    
    lel<T> * get_elem(int _idx){
      if(_idx<0||_idx>=dim){
	std::cerr << "lst<T>: Bad ID"<<std::endl;
	throw exception("Bad element id");
      }
      lel<T>* index = G;    
      for (register int k = 0; k < _idx&&index!=NULL; k++)
	index = index->d;      
      return index;
    }

    bool is_in(T o){
      for (lel<T>*index=G; index!=NULL;index=index->d)
	if(index->moi==o) return true;
      return false;
    }

    ///Returns a pointer to the iterator containing
    ///the object _object from the beginning of the container. 
    
    lel<T> * get_elem_from_obj(const T& _object){
      lel<T>* index;    
      for (index=G; index!=NULL;index=index->d)
	if(index->moi==_object)
	  return index;
      return NULL;      
    }

    T& get_obj(int i) { return operator[] (i); }

    ///Gets the element index on the list.
    virtual int get_id(const T& _o){
      int idx=0;
      lel<T>* it;

      for(it=G;it!=NULL;it=it->d){
	if(it->moi==_o) return idx;
	idx++;
      }
      return -1;
    }


    lel<T>* unlink_lel (lel<T>* _index)  {
      if(_index==NULL) return NULL;
  
      lel<T> *g, *d;
            
      g=_index->g;
      d=_index->d;
      
      if(g==NULL){
	if(d!=NULL){
	  G=d;
	  d->g=NULL;
	}
	else{	  
	  G=NULL;
	  D=NULL;
	}
      }
      else{
	if(d==NULL){
	  D=g;
	  g->d=NULL;
	}
	else{
	  g->d=d;
	  d->g=g;
	}	
      }
      
      dim--;
      return _index;
    }

    void delete_lel (lel<T>* _index)  {
      unlink_lel(_index);
      delete _index;
    }

    ///Moves element \par{_i} to the right.
    void a_d(int _i){swap(_i,_i+1);} 
    ///Moves element \par{_i} to the left.
    void a_g(int _i){swap(_i,_i-1);} 
    ///Swaps elements \par{_i} et \par{_j}.
    void swap(int _i, int _j)  {
      lel<T> * A, *B;
      T tmp;
      
      A=get_elem(_i);
      B=get_elem(_j);
      
      if(A==B)
	return;
      
      tmp=A->moi;
      A->moi=B->moi;
      B->moi=tmp;
      
    }
    
    ///Inserts a new object before/left of the current index \par{_i} object.
    T & pre_insert(int _i,T & _o)  {
      lel<T> * o=get_elem(_i);      
      return pre_insert(o,_o);
    }
    ///Inserts a new object after/right of the current index \par{_i} object.
    T & post_insert(int _i,T & _o)  {
      lel<T> * o=get_elem(_i);
      return post_insert(o,_o);
    }

    ///Inserts a new object before/left of the current itertor given by _pos.
    T & pre_insert(lel<T>* _pos,T & _o)  {
      if(_pos==NULL) throw exception("list nulpos");

      lel<T> *nouvel = new lel < T > (_o);
            
      nouvel->g = _pos->g; 
      nouvel->d = _pos; 
      
      if(_pos->g!=NULL) _pos->g->d=nouvel; else G=nouvel;
      _pos->g=nouvel;
      dim++;
      return _o;
    }
    ///Inserts a new object after/right of the current itertor given by _pos.
    T & post_insert(lel<T>* _pos,T & _o)  {
      
      if(_pos==NULL) throw exception("list nulpos");
      lel<T> *nouvel = new lel < T > (_o);

      nouvel->g = _pos; 
      nouvel->d = _pos->d; 
      
      if(_pos->d!=NULL) _pos->d->g=nouvel; else D=nouvel;
      _pos->d=nouvel;
      
      dim++;
      return _o;
    }
  
    
    T& change(lel<T>* _index, T& _new_obj){
      _index->set_obj(_new_obj);    
      return _new_obj;
    }
    
    T& change(lel<T>*_index, const T& _new_obj)  {
      _index->set_obj(_new_obj);    
      return _index->moi;
    }
    

    virtual bool retire(lel<T>* lt, bool deep=0)  {
      
      if(unlink_lel(lt)==NULL)
	return false;      

      delete lt;      
      return true;
    }


    virtual bool retire(int i, bool deep=0)  {
      if(i<0 || i>=dim) return false;
      
      lel<T>* lt=get_elem(i);
      
      if(unlink_lel(lt)==NULL)
	return false;
      
      delete lt;
      
      return true;
    }
    
    virtual bool retire_obj(T& _o, bool deep=0)  {

      lel<T>* lt=get_elem_from_obj(_o);
            
      if(unlink_lel(lt)==NULL)
	return false;
      
      delete lt;
      return true;
    }
    
    virtual bool retire_obj(const T& _o, bool deep=0)  {

      lel<T>* lt=get_elem_from_obj(_o);      
      
      if(unlink_lel(lt)==NULL)
	return false;

      delete lt;
      
      return true;
    }
  

    virtual bool retire_d(bool deep=0)  {
      if(D==NULL) return false;
      lel<T>* lt=D;
      
      if(unlink_lel(lt)==NULL)
	return false;
      
      delete lt;
      
      return true;
    }

    virtual bool retire_g(bool deep=0)  {
      if(G==NULL) return false;
      lel<T>* lt=G;
      
      if(unlink_lel(lt)==NULL)
	return false;
      
      delete lt;
      
      return true;
    }

    bool tt()  {
      lel<T> *t = D;
      while (t != NULL)
	{
	  D = D->g;
	  delete t;
	  t = D;
	}
      dim=0;
      D=NULL;
      G=NULL;
      return true;
    }


    virtual bool data_own(){return true;}
    virtual bool mem_create(int _nel){return false;}
    virtual bool mem_delete(){return tt();}

  };



  template < class T > class lst 
    : public olst<T> 
  {
   public:  

    using olst<T>::dim;
    using olst<T>::G;
    using olst<T>::D;
    using olst<T>::unlink_lel;
    using olst<T>::delete_lel;
    using olst<T>::change;
    using olst<T>::get_elem;
    using olst<T>::get_elem_from_obj;
    using olst<T>::tt;
    using olst<T>::add;


    ///The constructor of the list.
    lst():dp(false){}

    ///Copy constructor.
    lst(lst & _p):olst<T>(_p),dp(false) {
    }

    virtual ~lst(){
      if(dp) ttd(); else tt();
    }
    

    virtual bool retire(lel<T>* lt, bool deep=0)  {
      if(unlink_lel(lt)==NULL)
	return false;
      
      if(deep)
	delete lt->moi;
      
      delete lt;
      return true;
    }

    virtual bool retire(int i, bool deep=0)  {
      if(i<0 || i>=dim) return false;
      
      lel<T>* lt=get_elem(i);
      
      if(unlink_lel(lt)==NULL)
	return false;
      
      if(deep)
	delete lt->moi;
      
      delete lt;
      
      return true;
    }


    virtual bool retire_obj(T& _o, bool deep=0)  {
      lel<T>* lt=get_elem_from_obj(_o);

      if(unlink_lel(lt)==NULL)
	return false;
      
      if(deep)
	delete lt->moi;
      
      delete lt;
      
      return true;
    }
    
    virtual bool retire_obj(const T& _o, bool deep=0)  {
      lel<T>* lt=get_elem_from_obj(_o);

      if(unlink_lel(lt)==NULL)
	return false;      
      if(deep)
	delete lt->moi;
      
      delete lt;    
      return true;
    }


    virtual bool retire_g(bool deep=0)  {
      if(G==NULL) return false;
      lel<T>* lt=G;
      
      if(unlink_lel(lt)==NULL)
	return false;

      if(deep)
	delete lt->moi;

      delete lt;
      
      return true;
    }

    virtual bool retire_d(bool deep=0)  {
      if(D==NULL) return false;
      lel<T>* lt=D;
      
      if(unlink_lel(lt)==NULL)
	return false;

      if(deep)
	delete lt->moi;

      delete lt;
      
      return true;
    }
    
    T & replace(int _id, const  T& _new_obj, int _del_it=0){
      if (_id<0||_id >= dim){
	//      index=NULL;
	throw exception("index out of range");
      }
      
      lel<T>* lt=get_elem(_id);
      
      if(_del_it)
	delete lt->moi;
      
      return change(lt,_new_obj);
    }

    T & replace(T&  _old, T& _new, int _del)  {
      lel<T>* lt=get_elem_from_obj(_old);
      
      if(_del)
	delete lt->moi;
      
      return change(lt,_new);
    }

    virtual bool data_own(){
      return dp;
    }
    
    
    bool mem_create(int _nel){
      if(dim!=0) mem_delete();
      for(int i=0;i<_nel;i++) add(NULL);
      return true;
    }
    
    
    virtual bool mem_delete(){
      return dp ? ttd():tt();
    }
    
    bool ttd()  {
    //  if(!dp)return tt();
      lel<T> *t = D;
      while (t != NULL)
	{
	  D = D->g;
	  if(t->moi!=NULL)
	    delete t->moi;
	  delete t;
	  t = D;
	}
      dim=0;
      D=NULL;
      G=NULL;
      return true;
    }
    
    bool dp;
    
  };


  template <class BT, class T> class virtual_list{
  public:

    lst<BT>& srclist;
    //    typedef lel<T>* plel;
    int& dim;

    virtual_list(lst<BT>& srcl):
      srclist(srcl), 
      dim(srcl.dim){      
    }
    virtual ~virtual_list(){}

    ///Returns a reference to the object at 
    ///position \par{_id} from the beginning of the container. 
    
    virtual T& operator [] (int _id){
      return (T&)srclist[_id];
    }

    virtual T& add(const T& _o){
      return (T&)srclist.add(_o);
    }


    virtual T& adg(const T& _o) {
      return (T&)srclist.adg(_o);
    }

  };



}

#endif
