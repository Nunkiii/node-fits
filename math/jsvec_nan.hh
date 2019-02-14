#ifndef __JSVEC_HH__
#define __JSVEC_HH__

//#include <node.h>
//#include <node_object_wrap.h>
//#include <node_buffer.h>

#include <qk/vec.hh>
#include <qk/dcube.hh>

#include <nan.h>

namespace sadira{

  using namespace qk;
  //using namespace v8;
  using namespace Nan;
  
  //  using namespace node;

  void free_stream_buffer(char* b, void*x);
  
  template <typename T> class jsvec 
    : public ObjectWrap, public vec<T> {

  public:
    
    //static NAN_MODULE_INIT(Init) {
    static void init( v8::Local<v8::Object> exports, const char* class_name){
      //Isolate* isolate=exports->GetIsolate();
      v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
      
      tpl->InstanceTemplate()->SetInternalFieldCount(3);
      tpl->SetClassName(Nan::New(class_name).ToLocalChecked());
      
      SetPrototypeMethod(tpl, "length",length);
      SetPrototypeMethod(tpl, "get_value", get_value);
      SetPrototypeMethod(tpl, "set_value", set_value);

      constructor().Reset(GetFunction(tpl).ToLocalChecked());
	    

      //tpl->InstanceTemplate()->SetInternalFieldCount(1);
      
      // NODE_SET_PROTOTYPE_METHOD(tpl, "length",length);
      // NODE_SET_PROTOTYPE_METHOD(tpl, "get_value", get_value);
      // NODE_SET_PROTOTYPE_METHOD(tpl, "set_value", set_value);
      
      // constructor.Reset(isolate, tpl->GetFunction());

      // exports->Set(String::NewFromUtf8(isolate, class_name),tpl->GetFunction());
		   
    }
    static v8::Local<v8::Object> NewInstance(int argc, v8::Local<v8::Value>  argv[]){
      v8::Local<v8::Function> cons = Nan::New(constructor());
      return Nan::NewInstance(cons, argc, argv).ToLocalChecked();


      // Isolate* isolate = args.GetIsolate();

      // const unsigned argc = 1;
      // v8::Local<Value> argv[argc] = { args[0] };
      // v8::Local<Function> cons = Local<Function>::New(isolate, constructor);
      // v8::Local<Object> instance = cons->NewInstance(argc, argv);

      // args.GetReturnValue().Set(instance);
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
    
    static NAN_METHOD(New) {

      //Isolate* isolate = args.GetIsolate();
      //v8::Local<Context> context = isolate->GetCurrentContext();
      
      if (info.IsConstructCall()) {
	// Invoked as constructor: `new MyObject(...)`
	int value = (int) info[0]->IsUndefined() ? 0 : info[0]->NumberValue();
	jsvec* obj = new jsvec(value);
	obj->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
      } else {
	// Invoked as plain function `MyObject(...)`, turn into construct call.
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = { info[0] };
	v8::Local<v8::Function> cons = Nan::New(constructor()); //Local<Function>::New(isolate, constructor);
	//v8::Local<Object> result =cons->NewInstance(context, argc, argv).ToLocalChecked();
	info.GetReturnValue().Set( Nan::NewInstance(cons, argc, argv).ToLocalChecked());//cons->NewInstance(argc, argv));
      }
      
    }

  public:
    
    static NAN_METHOD(get_value) {

      //      Isolate* isolate = args.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(info.Holder());
      
      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New("Wrong number of arguments").ToLocalChecked());
      }
      
      if (!info[0]->IsNumber()) {
	Nan::ThrowError(Nan::New("Argument must be a Number").ToLocalChecked());
	return;
      }
      
      double d_num=(double) info[0]->NumberValue();
      if(d_num<0 || d_num>obj->dim-1){
	Nan::ThrowError(Nan::New("Argument must be a positive integer >=0 and < vector dimension.").ToLocalChecked());
	return;
      }

      //double v = (double) obj->c[(int)d_num]*1.0;
      
      //MINFO << "Access element " << d_num << " = " << v << endl;
      

      info.GetReturnValue().Set(Nan::New((double) obj->c[(int)d_num]*1.0));
      //info.GetReturnValue().Set(Number::New(isolate, v ));
    }

    static void set_value(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      //Isolate* isolate = info.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(info.Holder());
      
      if (info.Length() < 2) {
	Nan::ThrowError(Nan::New("2 arguments required!").ToLocalChecked());
	return;
      }
      
      if (!info[0]->IsNumber()) {
	Nan::ThrowError(Nan::New( "Argument 1 must be a Number").ToLocalChecked());
	return;
      }

      if (!info[1]->IsNumber()) {
	Nan::ThrowError(Nan::New("Argument 2 must be a Number").ToLocalChecked());
	return;
      }
      
      double d_num=(double) info[0]->NumberValue();
      double d_val=(double) info[1]->NumberValue();
      
      if(d_num<0 || d_num>obj->dim-1){
	Nan::ThrowError(Nan::New("Argument 1 must be a positive integer >=0 and < vector dimension.").ToLocalChecked());
	return;
      }

      obj->c[(int)d_num]=d_val;
      
      info.GetReturnValue().Set(info.Holder());
    }
    
    static void length(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      //Isolate* isolate = info.GetIsolate();
      jsvec* obj = ObjectWrap::Unwrap<jsvec>(info.Holder());
      
      info.GetReturnValue().Set(Nan::New((uint32_t) obj->dim));
    }

  private:
    
    static inline Nan::Persistent<v8::Function> & constructor() {
      static Nan::Persistent<v8::Function> my_constructor;
      return my_constructor;
    }

    
    // static Nan::Persistent<v8::Function> constructor;
    
  };
  
  //  template <typename T> Nan::Persistent<v8::Function> jsvec<T>::constructor;
  
}

#endif
