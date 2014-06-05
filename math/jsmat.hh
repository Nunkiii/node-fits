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
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "get_data", get_data);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "gen_pngtile",gen_pngtile);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "width",width);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "height",height);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "resize",resize);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "crop",crop);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "copy",copy);
      NODE_SET_PROTOTYPE_METHOD(s_ctm, "extend",extend);
      
      target->Set(String::NewSymbol(class_name), s_ctm->GetFunction());
      constructor = Persistent<Function>::New(tpl->GetFunction());
    }

  private:
    

  public:

    jsmat (int _d0=0, int _d1=0)  
      :mat<T>(_d0,_d1){
      //cout << "New matrix "<< this <<" D="<<dims[0]<<"," << dims[1] << " D="<< dim << " datap="<< ((void*)data_pointer()) << endl;
    }    
    jsmat (const jsmat<T> & m):mat<T>(m){}   
    
    virtual ~jsmat(){}

    using cnt::dim;
    using cnt::data_pointer;
    using mat<T>::dims;
    using mat<T>::redim;
    using mat<T>::operator=;
    

    const jsmat<T>& operator=(const jsmat<T>& m){
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


    static v8::Handle<v8::Value> copy(const v8::Arguments& args) {
      v8::HandleScope scope;
      return scope.Close(Undefined());
    }

    static v8::Handle<v8::Value> crop(const v8::Arguments& args) {
      v8::HandleScope scope;
      jsmat<T>* obj = NULL;
      obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());

      if (args.Length() < 1) {
	ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	return scope.Close(Undefined());
      }
      Local<Object> params = Local<Object>::Cast(args[0]->ToObject()); //->Get(String::New("cuts")));

      Handle<Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(String::NewSymbol("x")); if(hv != Undefined()) r[0]=hv->ToNumber()->Value();
      hv=params->Get(String::NewSymbol("y")); if(hv != Undefined()) r[1]=hv->ToNumber()->Value();
      hv=params->Get(String::NewSymbol("w")); if(hv != Undefined()) r[2]=hv->ToNumber()->Value();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(String::NewSymbol("h")); if(hv != Undefined()) r[3]=hv->ToNumber()->Value();else r[3]=obj->dims[1]-r[1];



      cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;
      
      if(obj->is_in(r)){
	//v8::String::Utf8Value fffu(hv->ToString());
	cout << "OK rect is inside !"<< endl;
	if(r[2]==obj->dims[0]&&r[3]==obj->dims[1])
	  return scope.Close(args.This());

	jsmat<T>* cropped=new jsmat<T>(*obj);
	obj->redim(r[2],r[3]);
	for(int j=0;j<r[3];j++)
	  for(int i=0;i<r[2];i++)
	    (*obj)(j,i)=(*cropped)(j+r[1],i+r[0]);
	delete cropped;

	  //cropped->extract_from(*obj,r.c);
	  //*obj=*cropped;
	
	
      }else{
	ThrowException(Exception::TypeError(String::New("Invalid area for cropping")));
	return scope.Close(Undefined());
      }



      //
      
      return scope.Close(args.This());
    }


    static v8::Handle<v8::Value> extend(const v8::Arguments& args) {

      cout << "Extend !" << endl;
      v8::HandleScope scope;
      jsmat<T>* obj = NULL;
      cout << "Extend !" << endl;
      obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());

      if (args.Length() < 1) {
	ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	return scope.Close(Undefined());
      }
      cout << "Extend !" << endl;
      Local<Object> params = Local<Object>::Cast(args[0]->ToObject()); //->Get(String::New("cuts")));

      Handle<Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(String::NewSymbol("x")); if(hv != Undefined()) r[0]=hv->ToNumber()->Value();
      hv=params->Get(String::NewSymbol("y")); if(hv != Undefined()) r[1]=hv->ToNumber()->Value();
      hv=params->Get(String::NewSymbol("w")); if(hv != Undefined()) r[2]=hv->ToNumber()->Value();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(String::NewSymbol("h")); if(hv != Undefined()) r[3]=hv->ToNumber()->Value();else r[3]=obj->dims[1]-r[1];

      cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;

      if(r[2]<=obj->dims[0] || r[3]<=obj->dims[1] ){
	ThrowException(Exception::TypeError(String::New("Invalid area for cropping")));
	return scope.Close(Undefined());
      }
      
      jsmat<T>* cropped=new jsmat<T>(*obj);
      obj->redim(r[2],r[3]);
      obj->set_all(0);
      int cd[2]={cropped->dims[0],cropped->dims[1]};
      int sp[2]={(int)((r[2]-cd[0])*.5),(int)((r[3]-cd[1])*.5)};
      for(int j=0;j<cd[1];j++)
	for(int i=0;i<cd[0];i++)
	  (*obj)(j+sp[1],i+sp[0])=(*cropped)(j+r[1],i+r[0]);
      delete cropped;
      
      //cropped->extract_from(*obj,r.c);
	  //*obj=*cropped;
	

      


      //
      
      return scope.Close(args.This());
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

    static Handle<Value> get_data(const Arguments& args) {

      if (args.Length() > 1) {
	//	ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	//	return scope.Close(Undefined());
      }
      
      jsmat<T>* obj = ObjectWrap::Unwrap<jsmat<T> >(args.This());
      //cout << "gen histo for " << obj->file_name << endl;
      
      HandleScope scope;
      //      Handle<String> result = v8::String::New("Hello");
      //Handle<node::ArrayBuffer> result = node::ArrayBuffer::New(15);

      /*
      node::Buffer *slowBuffer = node::Buffer::New(512*512*4);
      
      // Buffer:Data gives us a yummy void* pointer to play with to our hearts
      // content.
      
      for(int i=0;i<512;i++){
      for(int j=0;j<512;j++){
	float v=(float) (*obj)(i,j);
	float* b=(float*)node::Buffer::Data(slowBuffer);
	memcpy(b+i*512+j, &v, 4);
      }
      }
      */

      node::Buffer *slowBuffer = node::Buffer::New(obj->dim*4);
      
      // Buffer:Data gives us a yummy void* pointer to play with to our hearts
      // content.
      
      for(int i=0;i<obj->dim;i++){
	float v=(float) obj->c[i];
	float* b=(float*)node::Buffer::Data(slowBuffer);
	memcpy(b+i, &v, 4);
      }

      
      
      
      // Now we need to create the JS version of the Buffer I was telling you about.
      // To do that we need to actually pull it from the execution context.
      // First step is to get a handle to the global object.
      v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();

      // Now we need to grab the Buffer constructor function.
      v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));

      // Great. We can use this constructor function to allocate new Buffers.
      // Let's do that now. First we need to provide the correct arguments.
      // First argument is the JS object Handle for the SlowBuffer.
      // Second arg is the length of the SlowBuffer.
      // Third arg is the offset in the SlowBuffer we want the .. "Fast"Buffer to start at.
      v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer->handle_, v8::Integer::New(obj->dim*4), v8::Integer::New(0) };

      // Now we have our constructor, and our constructor args. Let's create the 
      // damn Buffer already!
      v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

      // This Buffer can now be provided to the calling JS code as easy as this:
      return scope.Close(actualBuffer);


      static Persistent<Function> uint16_array_constructor;

      if (uint16_array_constructor.IsEmpty()) {
	Local<Object> global = Context::GetCurrent()->Global();
	Local<Value> val = global->Get(String::New("Float32Array"));
	assert(!val.IsEmpty() && "type not found: Float32Array");
	assert(val->IsFunction() && "not a constructor: Float32Array");
	uint16_array_constructor = Persistent<Function>::New(val.As<Function>());
      }

      Local<Value> size = Integer::NewFromUnsigned(obj->dim);
      Local<Object> array = uint16_array_constructor->NewInstance(1, &size);


      if (array->GetIndexedPropertiesExternalArrayDataType() != kExternalFloatArray){
	ThrowException(Exception::TypeError(String::New("ExternalArrayDataType() != kExternalFloatArray")));
	return scope.Close(Undefined());
      }
      
      //int len = array->GetIndexedPropertiesExternalArrayDataLength();
      
      float* data = static_cast<float*>(array->GetIndexedPropertiesExternalArrayData());
      
      printf("first bytes : %d %d\n", (int) ( ((char*)obj->c)[0]), (int) ( ((char*)obj->c)[1]));
      for(int i=0;i<obj->dim;i++) data[i]=obj->c[i];
      
      return scope.Close(array);
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
