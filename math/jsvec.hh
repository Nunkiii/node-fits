#ifndef __JSVEC_HH__
#define __JSVEC_HH__

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

#include <qk/vec.hh>
#include <qk/dcube.hh>


namespace sadira{

  using namespace qk;
  using namespace v8;
  using namespace node;

  void free_stream_buffer(char* b, void*x);
  
  template <typename T> class jsvec 
    : public ObjectWrap, public vec<T> {

  public:
    
    static void init(Local<Object> exports, const char* class_name){
      Isolate* isolate=exports->GetIsolate();
      
      Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
      
      tpl->SetClassName(String::NewFromUtf8(isolate,class_name));
      tpl->InstanceTemplate()->SetInternalFieldCount(1);
      
      NODE_SET_PROTOTYPE_METHOD(tpl, "length",length);

      constructor.Reset(isolate, tpl->GetFunction());

      exports->Set(String::NewFromUtf8(isolate, class_name),tpl->GetFunction());
		   
    }
    static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args){
      Isolate* isolate = args.GetIsolate();

      const unsigned argc = 1;
      Local<Value> argv[argc] = { args[0] };
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      Local<Object> instance = cons->NewInstance(argc, argv);

      args.GetReturnValue().Set(instance);
    }

  private:
    
    using vec<T>::dim;
    using vec<T>::redim;
    using vec<T>::operator=;

    explicit jsvec (int _d=0)  
      :vec<T>(_d){
      //cout << "New vecrix "<< this <<" D="<<_d<<endl;
    }    
    virtual ~jsvec(){}
    
    static void New(const FunctionCallbackInfo<Value>& args) {

      Isolate* isolate = args.GetIsolate();

      if (args.IsConstructCall()) {
	// Invoked as constructor: `new MyObject(...)`
	int value = (int) args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	jsvec* obj = new jsvec(value);
	obj->Wrap(args.This());
	args.GetReturnValue().Set(args.This());
      } else {
	// Invoked as plain function `MyObject(...)`, turn into construct call.
	const int argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	args.GetReturnValue().Set(cons->NewInstance(argc, argv));
      }
      
    }
    
    static void length(const FunctionCallbackInfo<Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(args.Holder());

      args.GetReturnValue().Set(Number::New(isolate, obj->dim));
    }

    static Persistent<Function> constructor;
    
  };
  
  template <typename T> Persistent<Function> jsvec<T>::constructor;
  
}

#endif
