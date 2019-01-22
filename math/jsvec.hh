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
      NODE_SET_PROTOTYPE_METHOD(tpl, "get_value", get_value);
      NODE_SET_PROTOTYPE_METHOD(tpl, "set_value", set_value);
      
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
      Local<Context> context = isolate->GetCurrentContext();
      
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
	Local<Object> result =cons->NewInstance(context, argc, argv).ToLocalChecked();
	args.GetReturnValue().Set(result);//cons->NewInstance(argc, argv));
      }
      
    }

  public:
    
    static void get_value(const FunctionCallbackInfo<Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(args.Holder());
      
      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }
      
      if (!args[0]->IsNumber()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be a Number")));
	return;
      }
      
      double d_num=(double) args[0]->NumberValue();
      if(d_num<0 || d_num>obj->dim-1){
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be a positive integer >=0 and < vector dimension.")));
	return;
      }

      double v = (double) obj->c[(int)d_num]*1.0;
      
      //MINFO << "Access element " << d_num << " = " << v << endl;
      
      
      args.GetReturnValue().Set(Number::New(isolate, v ));
    }

    static void set_value(const FunctionCallbackInfo<Value>& args) {
      Isolate* isolate = args.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(args.Holder());
      
      if (args.Length() < 2) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"2 arguments required!")));
	return;
      }
      
      if (!args[0]->IsNumber()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument 1 must be a Number")));
	return;
      }

      if (!args[1]->IsNumber()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument 2 must be a Number")));
	return;
      }
      
      double d_num=(double) args[0]->NumberValue();
      double d_val=(double) args[1]->NumberValue();
      
      if(d_num<0 || d_num>obj->dim-1){
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument 1 must be a positive integer >=0 and < vector dimension.")));
	return;
      }

      obj->c[(int)d_num]=d_val;
      
      args.GetReturnValue().Set(args.Holder());
    }
    
    static void length(const FunctionCallbackInfo<Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(args.Holder());

      args.GetReturnValue().Set(Number::New(isolate, obj->dim));
    }

  private:
    static Persistent<Function> constructor;
    
  };
  
  template <typename T> Persistent<Function> jsvec<T>::constructor;
  
}

#endif
