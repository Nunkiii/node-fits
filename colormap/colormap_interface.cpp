#include <stdlib.h>
#include "colormap_interface.hh"

namespace sadira{
  
  void free_stream_buffer(char* b, void*x){
    free(b);
  }
  

  using namespace v8;

  Persistent<FunctionTemplate> colormap_interface::s_ct;
  

  Handle<Value> colormap_interface::set_cuts_histo(const Arguments& args){

  }

  Handle<Value> colormap_interface::set_cuts(const Arguments& args){
    
    HandleScope scope;
    
    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    colormap_interface* obj = ObjectWrap::Unwrap<colormap_interface>(args.This());

    Handle<Array> cutsa = Handle<Array>::Cast(args[0]);
    
    obj->cuts[0]=cutsa->Get(0)->ToNumber()->Value();
    obj->cuts[1]=cutsa->Get(1)->ToNumber()->Value();

    obj->rescale_colormap();
 
    return scope.Close(args.This());
    
  }
  
  
  void colormap_interface::rescale_colormap(){
    
    scaled_cmap = cmap;
    
    int ci;//,cc;
    //  lel<colormap_value<float>*> *lcv=cmap.G;
    
    for(ci=0;ci<scaled_cmap.dim;ci++){
      
      //    for(cc=0;cc<4;cc++)scaled_cmap[ci][cc]=(unsigned char)((*(**lcv))[cc]*255);
	
      scaled_cmap[ci][4]*=(cuts[1]-cuts[0]);
      scaled_cmap[ci][4]+=cuts[0];
      
      //lcv=lcv->d;
    }
    
  }
  
  
  Handle<Value> colormap_interface::set_colormap(const Arguments& args){
    
    HandleScope scope;
    
    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    colormap_interface* obj = ObjectWrap::Unwrap<colormap_interface>(args.This());
    Handle<Array> cmap_data = Handle<Array>::Cast(args[0]->ToObject());
    
    //  cout << "Setting colormap NC="<< cmap_data->Length() << endl;

    obj->cmap.ttd();
    qk::colormap_value<float>* cmv;
    unsigned int ncolors=cmap_data->Length();
    
    for(unsigned int c=0;c<ncolors;c++){
      cmv=new qk::colormap_value<float>();
      //  MINFO << "Reading color "<<c << endl;
      
      Local<Array> cmva = Local<Array>::Cast(cmap_data->Get(c));
	
      for(int cpn=0;cpn<5;++cpn){
	//  MINFO << "read " << cmap[c][cpn].asFloat() << endl;
	(*cmv)[cpn]=(float)cmva->Get(cpn)->ToNumber()->Value();
      }
      
      //    (*cmv)[4]*=(cuts[1]-cuts[0]);
      // (*cmv)[4]+=cuts[0];
      
      //    MINFO << "Reading color "<<c << "done" << endl;
      obj->cmap.add(cmv);
    }  

    
    return scope.Close(args.This());
    
  }
  
  
 
  void colormap_interface::init(v8::Handle<Object> target){
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    
    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(3);
    s_ct->SetClassName(String::NewSymbol("BaseClass"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "set_colormap", set_colormap);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "set_cuts", set_cuts);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "set_cuts_histo", set_cuts_histo);
    
    //target->Set(String::NewSymbol("colormap_interface"), s_ct->GetFunction());
    //s_ct->Set(String::NewSymbol("set_colormap"),FunctionTemplate::New(set_colormap_func)->GetFunction());
    //t->PrototypeTemplate()->Set(String::NewSymbol("set_cuts"),FunctionTemplate::New(set_cuts_func)->GetFunction());
  }
  
 
}
