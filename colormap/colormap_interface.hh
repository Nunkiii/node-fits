/*
- 2003 - 2014 - The Qk/Sadira project - P.G. Sprimont @ INAF, Bologna, IT. 
 */

#ifndef __COLORMAP_INTERFACE_HH__
#define __COLORMAP_INTERFACE_HH__

#include <node.h>
#include <qk/colormap.hh>

namespace sadira{

  using namespace v8;
  
  void free_stream_buffer(char* b, void*x);

  class colormap_interface  : public node::ObjectWrap {

  public:

    static void init(Handle<Object> target);
    
  protected:

    static Persistent<FunctionTemplate> s_ct;
    
    qk::colormap<float> cmap;
    qk::colormap_array<float> scaled_cmap;
    double cuts[2];

    void rescale_colormap();

    static Handle<Value> set_cuts_histo(const Arguments& args);
    static Handle<Value> set_cuts(const Arguments& args);
    static Handle<Value> set_colormap(const Arguments& args);
    
    static Handle<Value> New(const Arguments& args){
      HandleScope scope;
      return args.This();
    }
    
  };
}



#endif
