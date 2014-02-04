#ifndef __JSMAT_HH__
#define __JSMAT_HH__

#include <node.h>
#include <node_buffer.h>

#include <qk/mat.hh>
#include <qk/pngwriter.hh>
#include <qk/dcube.hh>

#include <colormap/colormap_interface.hh>

#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
FILE * open_memstream (char **buf, size_t *len);
FILE *fmemopen(void *buf, size_t size, const char *mode);
}
#endif
#endif


namespace sadira{

  using namespace qk;
  using namespace v8;
  using namespace node;

  void free_stream_buffer(char* b, void*x);
  
  template <typename T> class jsmat 
    : public colormap_interface, public mat<T> {
  public:

    static Persistent<FunctionTemplate> s_ctm;    
    static Persistent<Function> constructor;

    static void init(Handle<Object> target, const char* class_name){
      Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
      
      s_ctm = Persistent<FunctionTemplate>::New(tpl);
      
      s_ctm->Inherit(colormap_interface::s_ct); 
      
      s_ctm->InstanceTemplate()->SetInternalFieldCount(1);
      s_ctm->SetClassName(String::NewSymbol(class_name));
      
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "gen_histogram", gen_histogram);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "gen_pngtile",gen_pngtile);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "width",width);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "height",height);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "resize",resize);
      
      target->Set(String::NewSymbol(class_name), s_ctm->GetFunction());
      constructor = Persistent<Function>::New(tpl->GetFunction());
    }

  private:
    

  public:

    jsmat (int _d0=0, int _d1=0)  
      :mat<T>(_d0,_d1){
      //cout << "New matrix "<< this <<" D="<<dims[0]<<"," << dims[1] << " D="<< dim << " datap="<< ((void*)data_pointer()) << endl;
    }    
    jsmat (const mat<T> & m){
      operator = (m);
    }   
    
    virtual ~jsmat(){}

    using cnt::dim;
    using cnt::data_pointer;
    using mat<T>::dims;
    using mat<T>::redim;
    using mat<T>::operator=;
    

    const jsmat<T>& operator=(const mat<T>& m){
      mat<T>::operator=(m);
      return *this;
    }


    static Handle<Value> New(const Arguments& args) {
      HandleScope scope;
      int d0=0,d1=0;
      
      if (args.Length() >=2) {
	d0=args[0]->ToNumber()->Value();
	d1=args[1]->ToNumber()->Value();
      }

      jsmat<T>* obj = new jsmat<T>(d0,d1);
      obj->Wrap(args.This());

      //args.This()->Set(String::NewSymbol("id"), Number::New(12345));
      return args.This();
    }

    static Handle<Value> Instantiate(int d0=0, int d1=0) {
      HandleScope scope;
      int argc=0;
      Local<v8::Value> argv[2];

      cout << "Inst new matrix " << d0 << ", " << d1 << endl;
      
      if(d0>0 || d1>0){
	argv[0]=Local<v8::Value>::New(Number::New(d0));
	argv[1]=Local<v8::Value>::New(Number::New(d1));
	argc=2;
      }
      
      return scope.Close(constructor->NewInstance(argc,argv));
    }
    
    /*
    static Handle<Value> Instantiate(mat<T>& m) {
      HandleScope scope;

      Local<v8::Value> argv[1] = {
        Local<v8::Value>::New(String::New(name.c_str()))
      };

      return scope.Close(constructor->NewInstance(m));
    }
    */

    static v8::Handle<v8::Value> resize(const v8::Arguments& args) {
      v8::HandleScope scope;

      if (args.Length() < 2) {
	ThrowException(Exception::TypeError(String::New("You must pass width and height as parameters")));
	return scope.Close(Undefined());
      }

      jsmat<T>* obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      //cout << "OBJ= "<< obj <<" Img w="<<obj->dims[0]<<endl;
      MINFO << "redim matrix to " << args[0]->NumberValue() <<", " << args[1]->NumberValue() << endl;

      obj->redim(args[0]->NumberValue(), args[1]->NumberValue());
      return scope.Close(args.This());
    }

    static v8::Handle<v8::Value> width(const v8::Arguments& args) {
      v8::HandleScope scope;
      jsmat<T>* obj = NULL;
      obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      Handle<Value> w=Number::New(obj->dims[0]);
      return scope.Close(w);
    }

    static v8::Handle<v8::Value> height(const v8::Arguments& args) {
      v8::HandleScope scope;
      jsmat<T>* obj = NULL;
      obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      Handle<Value> w=Number::New(obj->dims[1]);
      return scope.Close(w);
    }

    
    static v8::Handle<v8::Value> gen_pngtile(const v8::Arguments& args) {

      v8::HandleScope scope;
    
      if (args.Length() < 1) {
	ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	return scope.Close(Undefined());
      }

      jsmat<T>* obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      
      Handle<Array> parameters = Handle<Array>::Cast(args[0]);
      Handle<Array> tile_size;

      if (args.Length() >= 1) {
	tile_size = Handle<Array>::Cast(args[1]);
      }else{

	tile_size=Array::New();
	
	tile_size->Set(0,Number::New(256));
	tile_size->Set(1,Number::New(256));
      }
      //  cout << " zoom = " << parameters->Get(0)->ToNumber()->Value() << endl;
      
      try{
	Handle<node::Buffer> bp = obj->gen_pngtile(parameters, tile_size);
	return scope.Close(bp->handle_);  
      }
      
      catch (qk::exception& e){
	//    MERROR << "Failed: "<<e.mess << endl;
	v8::ThrowException(v8::String::New(e.mess.c_str()));
      }
      
      //  v8::Handle<v8::Object> result = v8::Object::New();
      return scope.Close(Undefined());
    }
    
    
    v8::Handle<node::Buffer> gen_pngtile(Handle<Array>& parameters,Handle<Array>& tile_size) {
      
      //  float sbig_start[2];
      // float sbig_size[2];
      //int png_dims[2];
      int x_tile,y_tile,zoom;
      
      x_tile=parameters->Get(0)->ToNumber()->Value();
      y_tile=parameters->Get(1)->ToNumber()->Value();
      zoom=parameters->Get(2)->ToNumber()->Value();

      int ts[2]={ 
	(int) tile_size->Get(0)->ToNumber()->Value(),
	(int) tile_size->Get(1)->ToNumber()->Value(),
      };
      
      //cout << "Gen tile " << x_tile << ", " << y_tile << " zoom " << zoom << endl; 
      
      dcube<unsigned char> png_data(ts[0],ts[1],4);
      png_data.set_all(0);
      
      float scale_factor;
      
      //int* dims=last_image.dims;
      
      int longest_dim=0;
      if(dims[1]>dims[0]) longest_dim=1;
      
      //cout << "OK" << endl;  
      //longest_dim=0;
      
      scale_factor=dims[longest_dim]*1.0/png_data.dims[longest_dim];
      for(int z=0;z<zoom;z++) scale_factor/=2.0;
      
      //  cout << "Fits data :  " << img_hdu_dims.dim << " dimensions : ";
      // for(int d=0;d<img_hdu_dims.dim;d++) cout << img_hdu_dims[d] << ", ";
      // cout << endl;
      
    
      vec<float> tmpcol(4);
      int fpix[2];
      float value;

      for(int y=0;y<png_data.dims[1];y++)
	for(int x=0;x<png_data.dims[0];x++){
	  
	  fpix[0]=(x_tile*png_data.dims[0]+x)*scale_factor;
	  fpix[1]=(y_tile*png_data.dims[1]+y)*scale_factor;
	  
	  if(fpix[0]<0 || fpix[0]>=dims[0]
	     || fpix[1]<0 || fpix[1]>=dims[1]
	     ) value=cuts[1];
	  
	  else{
	    value = (float) (*this)(fpix[1],fpix[0]);
	    //fits_read_pix(f, TFLOAT, fpix.c, 1,&nulv,&value, &anynul, &fstat);
	    //report_fits_error();
	  }
	  
	  //value-=cuts[0];
	  
	  //      cout << "Getting colors cmd=" << scaled_cmap.dim << endl;
	  scaled_cmap.get_color(value, tmpcol);
	  
	  // value=(value-cuts[0])/(cuts[1]-cuts[0])*255.0; 
	  
	  // if(value<0) value=0;
	  // if(value>255) value=255;
	  //cout << "Color : ";
	  for(int c=0;c<4;c++) png_data(y,x,c)=(unsigned char) (tmpcol[c]*255.0);
	  
	  // png_data(y,x,0)=(unsigned char) (value);
	  // png_data(y,x,1)=(unsigned char) (value);
	  // png_data(y,x,2)=(unsigned char) (value);
	  // png_data(y,x,3)=255;
	}
      
      size_t stream_size;
      char* stream_data;
      FILE * fst = open_memstream(&stream_data, &stream_size);
      write_png_file(fst, png_data);
      fclose(fst);
      
      // POSSIBLE MEM LEAK ? Does v8::Buffer frees itself its mem content ?? 
      
      //  Buffer* bp =Buffer::New(stream_data, stream_size, free_stream_buffer, NULL)
      Buffer* bp =Buffer::New(stream_data, stream_size, free_stream_buffer, NULL);
      Handle<Buffer> hbp(bp);
      
      //  Handle<Buffer> bp = Buffer::New(stream_size);
      //memcpy(bp->data(), stream_data, stream_size);
      //  free(stream_data);
      return hbp;
    }

    
    static Handle<Value> gen_histogram(const Arguments& args) {
      
      jsmat<T>* obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      //cout << "gen histo for " << obj->file_name << endl;
      
      HandleScope scope;
      
      if (args.Length() < 1) {
	ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	return scope.Close(Undefined());
      }
      
      Local<Object> ar = args[0]->ToObject();
      Local<Array> cuts_array = Local<Array>::Cast(ar->Get(String::New("cuts")));
      
      double cuts[2];
      cuts[0]= cuts_array->Get(0)->ToNumber()->Value();
      cuts[1]= cuts_array->Get(1)->ToNumber()->Value();
      
      Handle<String> histo_csv_data = obj->create_image_histogram(cuts);
      //  cout << " HISTO OK : " << *(String::AsciiValue(histo_csv_data->ToString())) << endl;
      
      return scope.Close(histo_csv_data);
    }


    Handle<String> create_image_histogram(double* cuts){
      
      try{
	
	string result_string;
	//int fpix[2];

	//mat<float> imgdata(dims[0],dims[1]);
	//for(int i=0;i<dim;i++)imgdata[i]=(float)c[i];

	jsmat<T>& imgdata=*this;

	int nbins=200;
	
	float low= (int) cuts[0];
	float max= (int) cuts[1];
	
	if(low==-1||max==-1){
	  low=imgdata.min();
	  max=imgdata.max();
	}
	
	//cout << "OK2" << endl;
	
	float bsize=(max-low)/nbins;
	
	if(bsize<1){
	  bsize=1.0;
	  nbins=max-low;
	}
	
	vec<unsigned int> histo(nbins);
	histo.set_all(0);
	
	for(int d=0;d<imgdata.dim;d++) 
	  if(imgdata[d]>=low&&imgdata[d]<max){
	    int bid=(int)( (imgdata[d]-low)/bsize);
	    if(bid>=0&&bid<histo.dim)
	      histo[bid]++; 
	  }
	
	//cout << "OK3" << endl;
	
	result_string="pixvalue\tndata\n";
	
	char sb[256];
	
	for(int i=0;i<nbins;i++){
	  sprintf(sb,"%g\t%g\n",low+(i+.5)*bsize, histo[i]*1.0);
	  result_string+=sb;
	}
	
	
	Handle<String> result = v8::String::New(result_string.c_str());
	
	//cout << "OK5" << endl;
	return result;
      }
      
      catch (qk::exception& e){
	//cout << "Catch qk exception " << endl;
	v8::ThrowException(v8::String::New(e.mess.c_str()));
      }
      
      
      
      return Handle<String>();
    }
    
    
    
    
  };

  template <class T> inline jsmat<T>* jsmat_unwrap(v8::Handle<v8::Object> handle) {
    
    
    assert(!handle.IsEmpty());
    assert(handle->InternalFieldCount() > 0);
    // Cast to ObjectWrap before casting to T.  A direct cast from void
    // to T won't work right when T has more than one base class.
    void* ptr = handle->GetPointerFromInternalField(0);
    ObjectWrap* wrap = static_cast<ObjectWrap*>(ptr);
    
    return static_cast<jsmat<T> *>(wrap);
  }


}

#endif
