#ifndef __COLORMAP_INTERFACE_HH__
#define __COLORMAP_INTERFACE_HH__

#include <node.h>
#include <qk/colormap.hh>

namespace sadira{

  using namespace v8;

  void free_stream_buffer(char* b, void*x);

  class colormap_interface  : public node::ObjectWrap {


  public:
    static void init(v8::Handle<Object> target);

  protected:

    static Persistent<FunctionTemplate> s_ct;
    
    qk::colormap<float> cmap;
    qk::colormap_array<float> scaled_cmap;
    double cuts[2];

    void set_cuts(v8::Handle<v8::Array>& cutsa);
    void rescale_colormap();
    void set_colormap(v8::Handle<v8::Array>& colormap) ;

    static v8::Handle<Value> set_cuts_func(const v8::Arguments& args);
    static v8::Handle<v8::Value> set_colormap_func(const v8::Arguments& args);
 

    static v8::Handle<Value> New(const v8::Arguments& args){
      HandleScope scope;
      return args.This();
    }
        
  };

}



#endif
