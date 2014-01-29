#ifndef __JSVEC_HH__
#define __JSVEC_HH__

#include <node.h>
#include <node_buffer.h>

#include <qk/vec.hh>
#include <qk/dcube.hh>

#include <colormap/colormap_interface.hh>

namespace sadira{

  using namespace qk;
  using namespace v8;
  using namespace node;

  void free_stream_buffer(char* b, void*x);
  
  template <typename T> class jsvec 
    : public ObjectWrap, public vec<T> {
    //class jsvec : public ObjectWrap {
  public:
    
    static Persistent<FunctionTemplate> s_ctm;    
    //static Persistent<Function> constructor;
    
    static void init(Handle<Object> target, const char* class_name){
      Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
      s_ctm = Persistent<FunctionTemplate>::New(tpl);
      
      //s_ctm->Inherit(colormap_interface::s_ct); 
      
      s_ctm->InstanceTemplate()->SetInternalFieldCount(1);
      s_ctm->SetClassName(String::NewSymbol(class_name));
      
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "length",length);
      //NODE_SET_PROTOTYPE_METHOD(s_ctm, "resize",resize);
      
      target->Set(String::NewSymbol(class_name), s_ctm->GetFunction());
      //constructor = Persistent<Function>::New(tpl->GetFunction());
    }

  private:

    jsvec (int _d=0)  
      :vec<T>(_d)
    {
      cout << "New vecrix "<< this <<" D="<<_d<<endl;
    }    

    // jsvec (const vec<T> & m){
    //   operator = (m);
    // }   
    
    virtual ~jsvec(){}


    static Handle<Value> New(const Arguments& args) {
      HandleScope scope;

      //jsvec<T>* obj = new jsvec<T>();
      jsvec* obj = new jsvec();

      obj->Wrap(args.This());

      args.This()->Set(String::NewSymbol("id"), Number::New(12345));
      return args.This();
    }


    static v8::Handle<v8::Value> length(const v8::Arguments& args) {
    
      v8::HandleScope scope;


      // jsvec<T>* obj = NULL;
      // obj = ObjectWrap::Unwrap<jsvec<T> >(args.This());

      jsvec* obj = NULL;
      obj = ObjectWrap::Unwrap<jsvec>(args.This());
      
      cout << "Unwrapped VEC... ptr" << obj <<endl;
      obj->redim(obj->dim+1);
      //      cout << "OBJ= "<< obj <<" D="<<obj->dim<<endl;
      //obj->redim(5);
      //      cout << "NEWOBD ="<<obj->dim<<endl;
      // jsvec<float> jsf(10,10);
      // cout << "jsf w="<<jsf.dims[0]<<endl;

      Handle<Value> w=Number::New(obj->dim);

      return scope.Close(w);
    }
    

  public:

    using vec<T>::dim;
    using vec<T>::redim;
    using vec<T>::operator=;
    
  };
}

#endif
