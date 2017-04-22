#ifndef __JSMAT_HH__
#define __JSMAT_HH__

#include <qk/mat.hh>
#include <math/jsvec.hh>
#include <qk/pngwriter.hh>
#include <qk/jpeg_writer.hh>
#include <qk/dcube.hh>
#include <qk/colormap.hh>

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
    : public ObjectWrap, public mat<T> {

  public:

    static Persistent<Function> constructor;

    static void init(Local<Object> exports, const char* class_name){
      Isolate* isolate = exports->GetIsolate();
      
      Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
      
      tpl->SetClassName(String::NewFromUtf8(isolate,class_name));
      tpl->InstanceTemplate()->SetInternalFieldCount(11);
      
      NODE_SET_PROTOTYPE_METHOD(tpl, "length",length);
      NODE_SET_PROTOTYPE_METHOD(tpl, "width",width);
      NODE_SET_PROTOTYPE_METHOD(tpl, "height",height);
      NODE_SET_PROTOTYPE_METHOD(tpl, "resize",resize);
      NODE_SET_PROTOTYPE_METHOD(tpl, "crop",crop);
      NODE_SET_PROTOTYPE_METHOD(tpl, "copy",copy);
      NODE_SET_PROTOTYPE_METHOD(tpl, "extend",extend);
      NODE_SET_PROTOTYPE_METHOD(tpl, "swapx",swapx);
      NODE_SET_PROTOTYPE_METHOD(tpl, "swapy",swapy);
      NODE_SET_PROTOTYPE_METHOD(tpl, "set_colormap", set_colormap);
      NODE_SET_PROTOTYPE_METHOD(tpl, "set_cuts", set_cuts);
      NODE_SET_PROTOTYPE_METHOD(tpl, "set_cuts_histo", set_cuts_histo);
      NODE_SET_PROTOTYPE_METHOD(tpl, "histogram", gen_histogram);
      NODE_SET_PROTOTYPE_METHOD(tpl, "get_data", get_data);
      NODE_SET_PROTOTYPE_METHOD(tpl, "set_data", set_data);
      NODE_SET_PROTOTYPE_METHOD(tpl, "tile",tile);

      constructor.Reset(isolate, tpl->GetFunction());
      exports->Set(String::NewFromUtf8(isolate, class_name),tpl->GetFunction());
    }
    
    qk::colormap<float> cmap;
    qk::colormap_array<float> scaled_cmap;
    double cuts[2];
    
  private:

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args) {
      
      Isolate* isolate = args.GetIsolate();

      if (args.IsConstructCall()) {
	// Invoked as constructor: `new MyObject(...)`
	int d0= (int) args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	int d1= (int) args[1]->IsUndefined() ? 0 : args[1]->NumberValue();
	jsmat* obj = new jsmat(d0,d1);
	obj->Wrap(args.This());
	args.GetReturnValue().Set(args.This());
      } else {
	// Invoked as plain function `MyObject(...)`, turn into construct call.
	const int argc = 2;
	Local<Value> argv[argc] = { args[0], args[1] };
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	args.GetReturnValue().Set(cons->NewInstance(argc, argv));
      }
      
    }

  public:
    explicit jsmat (int _d0=0, int _d1=0)  
      :mat<T>(_d0,_d1){
      //cout << "New matrix "<< this <<" D="<<dims[0]<<"," << dims[1] << " D="<< dim << " datap="<< ((void*)data_pointer()) << endl;
    }    
    jsmat (const jsmat<T> & m):mat<T>(m){}   

  public:

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

    static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args){
      Isolate* isolate = args.GetIsolate();
      
      const unsigned argc = 2;
      Local<Value> argv[argc] = { args[0], args[1] };
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      Local<Object> instance = cons->NewInstance(argc, argv);

      args.GetReturnValue().Set(instance);
    }

    
    static void length(const FunctionCallbackInfo<Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      args.GetReturnValue().Set(Number::New(isolate, obj->dim));
    }

    static void set_cuts_histo(const v8::FunctionCallbackInfo<v8::Value>& args){}
    
    static void set_cuts(const v8::FunctionCallbackInfo<v8::Value>& args){

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());
      
      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
	return;
      }

      if(! args[0]->IsArray()){
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be an Array")));
	return;
      }
      Handle<Array> cutsa = Handle<Array>::Cast(args[0]);
      
      obj->cuts[0]=cutsa->Get(0)->NumberValue();
      obj->cuts[1]=cutsa->Get(1)->NumberValue();
      obj->rescale_colormap();

      args.GetReturnValue().Set(args.Holder());

    }
    
  
    void rescale_colormap(){

      scaled_cmap = cmap;
      
      int ci;

      for(ci=0;ci<scaled_cmap.dim;ci++){
	scaled_cmap[ci][4]*=(cuts[1]-cuts[0]);
	scaled_cmap[ci][4]+=cuts[0];
      }
      
    }
    
    
    static void set_colormap(const v8::FunctionCallbackInfo<v8::Value>& args){

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }

      if(! args[0]->IsArray()){
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be an Array")));
	return;
      }

      Handle<Array> cmap_data = Handle<Array>::Cast(args[0]);
      
      obj->cmap.ttd();
      qk::colormap_value<float>* cmv;
      unsigned int ncolors=cmap_data->Length();
      
      for(unsigned int c=0;c<ncolors;c++){
	cmv=new qk::colormap_value<float>();
	//  MINFO << "Reading color "<<c << endl;
	Local<Array> cmva = Local<Array>::Cast(cmap_data->Get(c));
	for(int cpn=0;cpn<5;++cpn){
	  //  MINFO << "read " << cmap[c][cpn].asFloat() << endl;
	  (*cmv)[cpn]=(float)cmva->Get(cpn)->NumberValue();
	}
	
	obj->cmap.add(cmv);
      }  
      args.GetReturnValue().Set(args.Holder());
    }
    
    static void resize(const v8::FunctionCallbackInfo<v8::Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      if (args.Length() < 2) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"You must pass width and height as parameters")));
	return;
      }
      if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments must be numbers")));
	return;
      }
      
      obj->redim(args[0]->NumberValue(), args[1]->NumberValue());
      args.GetReturnValue().Set(args.Holder());
    }

    static void width(const v8::FunctionCallbackInfo<v8::Value>& args) {
      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());
      Local<Number> d=Number::New(isolate, obj->dims[0]);
      args.GetReturnValue().Set(d);
    }

    static void height(const v8::FunctionCallbackInfo<v8::Value>& args) {
      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());
      Local<Number> d=Number::New(isolate, obj->dims[1]);
      args.GetReturnValue().Set(d);
    }


    static void copy(const v8::FunctionCallbackInfo<v8::Value>& args) {}

    static void crop(const v8::FunctionCallbackInfo<v8::Value>& args) {
      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }
      if (!args[0]->IsObject()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be an object")));
	return;
      }

      Local<Object> params = Local<Object>::Cast(args[0]->ToObject()); //->Get(String::NewFromUtf8(isolate,"cuts")));
      
      Handle<Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(String::NewFromUtf8(isolate,"x"));if(hv != Undefined(isolate)) r[0]=hv->NumberValue();
						     
      
      hv=params->Get(String::NewFromUtf8(isolate,"y")); if(hv != Undefined(isolate)) r[1]=hv->NumberValue();
      hv=params->Get(String::NewFromUtf8(isolate,"w")); if(hv != Undefined(isolate)) r[2]=hv->NumberValue();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(String::NewFromUtf8(isolate,"h")); if(hv != Undefined(isolate)) r[3]=hv->NumberValue();else r[3]=obj->dims[1]-r[1];

      //cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;
      
      if(obj->is_in(r)){
	//v8::String::Utf8Value fffu(hv->ToString());
	cout << "OK rect is inside !"<< endl;
	if(r[2]==obj->dims[0]&&r[3]==obj->dims[1]){
	  args.GetReturnValue().Set(args.Holder());
	  return;
	}
	
	jsmat<T>* cropped=new jsmat<T>(*obj);
	obj->redim(r[2],r[3]);
	for(int j=0;j<r[3];j++)
	  for(int i=0;i<r[2];i++)
	    (*obj)(j,i)=(*cropped)(j+r[1],i+r[0]);
	delete cropped;

	  //cropped->extract_from(*obj,r.c);
	  //*obj=*cropped;
	
	
      }else{
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Invalid area for cropping")));
	return;
      }
      args.GetReturnValue().Set(args.Holder());
    }


    static void extend(const v8::FunctionCallbackInfo<v8::Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }
      //cout << "Extend !" << endl;
      if (!args[0]->IsObject()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be an object")));
	return;
      }

      Local<Object> params = Local<Object>::Cast(args[0]->ToObject()); //->Get(String::NewFromUtf8(isolate,"cuts")));

      Handle<Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(String::NewFromUtf8(isolate,"x")); if(hv != Undefined(isolate)) r[0]=hv->NumberValue();
      hv=params->Get(String::NewFromUtf8(isolate,"y")); if(hv != Undefined(isolate)) r[1]=hv->NumberValue();
      hv=params->Get(String::NewFromUtf8(isolate,"w")); if(hv != Undefined(isolate)) r[2]=hv->NumberValue();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(String::NewFromUtf8(isolate,"h")); if(hv != Undefined(isolate)) r[3]=hv->NumberValue();else r[3]=obj->dims[1]-r[1];

      //cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;

      if(r[2]<=obj->dims[0] || r[3]<=obj->dims[1] ){
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Invalid area for cropping")));
	return;
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

      args.GetReturnValue().Set(args.Holder());
    }

    static void tile(const v8::FunctionCallbackInfo<v8::Value>& args) {
      
      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());
      
      if (args.Length() < 1) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }
      
      if (!args[0]->IsObject()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be an Object")));
	return;
      }
      
      Handle<Array> parameters = Handle<Array>::Cast(args[0]);

      Handle<Array> tile_coord = Handle<Array>::Cast(parameters->Get(String::NewFromUtf8(isolate,"tile_coord")));
      Handle<Array> tile_size = Handle<Array>::Cast(parameters->Get(String::NewFromUtf8(isolate,"tile_size")));
      Handle<Integer> zoom_par = Handle<Integer>::Cast(parameters->Get(String::NewFromUtf8(isolate,"zoom")));
      Handle<String> type_par= Handle<String>::Cast(parameters->Get(String::NewFromUtf8(isolate,"type")));

      int x_tile,y_tile,zoom, ts[2];

      if(tile_coord->IsUndefined()) {
	x_tile=0;
	y_tile=0;
      }else{
	x_tile= Handle<Integer>::Cast(tile_coord->Get(0))->Value();
	y_tile= Handle<Integer>::Cast(tile_coord->Get(1))->Value();
      }

      if(tile_size->IsUndefined()) {
	ts[0]=ts[1]=256;
      }else{
	ts[0]=Handle<Integer>::Cast(tile_size->Get(0))->Value();
	ts[1]=Handle<Integer>::Cast(tile_size->Get(1))->Value();
      }
      
      if(zoom_par->IsUndefined()) {
	zoom=0;
      }else
	zoom=Handle<Integer>::Cast(zoom_par)->Value();

      string type ="jpeg";
      if(! type_par->IsUndefined()){
      	v8::String::Utf8Value s(type_par->ToString());
	type=*s;
      }

      //MINFO << "Building tile sz " << ts[0] << ", " << ts[1] << " coord " << x_tile << ", " << y_tile << " z= " << zoom << " type " << type << endl;

      //return scope.Close(args.This());
      
      try{
	//Handle<node::Buffer> bp = obj->gen_pngtile(parameters, tile_size);
	int depth= (type == "png" ) ? 4 : 3;
	int* dims=obj->dims;

	dcube<unsigned char> png_data(ts[0],ts[1],depth);
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
	       ) value=obj->cuts[1];
	    
	    else{
	      value = (float) (*obj)(fpix[1],fpix[0]);
	      //fits_read_pix(f, TFLOAT, fpix.c, 1,&nulv,&value, &anynul, &fstat);
	      //report_fits_error();
	    }
	    
	    //value-=obj->cuts[0];
	    
	    //      cout << "Getting colors cmd=" << scaled_cmap.dim << endl;
	    obj->scaled_cmap.get_color(value, tmpcol);
	    
	    // value=(value-obj->cuts[0])/(obj->cuts[1]-obj->cuts[0])*255.0; 
	    
	    // if(value<0) value=0;
	    // if(value>255) value=255;
	    //cout << "Color : ";
	    for(int c=0;c<depth;c++) png_data(png_data.dims[1]-y-1,x,c)=(unsigned char) (tmpcol[c]*255.0);
	    
	  // png_data(y,x,0)=(unsigned char) (value);
	  // png_data(y,x,1)=(unsigned char) (value);
	  // png_data(y,x,2)=(unsigned char) (value);
	  // png_data(y,x,3)=255;
	  }
	
	size_t stream_size;
	char* stream_data;
	FILE * fst = open_memstream(&stream_data, &stream_size);
	if(type == "png")
	  write_png_file(fst, png_data);
	else
	  write_jpeg_file(fst, png_data);
	fclose(fst);
	
	// POSSIBLE MEM LEAK ? Does v8::Buffer frees itself its mem content ?? 
	
	//  Buffer* bp =Buffer::New(stream_data, stream_size, free_stream_buffer, NULL)
	//Buffer* bp;
	
	MaybeLocal<Object> mbp=Buffer::New(isolate, stream_data, stream_size);
	Local<Object> bp=mbp.ToLocalChecked();
	
	memcpy(Buffer::Data(bp), stream_data, stream_size);
	
	//Handle<Buffer> hbp(bp);
	
	args.GetReturnValue().Set(bp);  
      }
      
      catch (qk::exception& e){
	//    MERROR << "Failed: "<<e.mess << endl;
	isolate->ThrowException(v8::String::NewFromUtf8(isolate,e.mess.c_str()));
      }
    }

    static void swapx(const v8::FunctionCallbackInfo<v8::Value>& args) {
      //Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      mat<T> tmp(*obj);
      int* dims=obj->dims;
      for(int j=0;j<dims[1];j++){
	for(int i=0;i<dims[0];i++){
	  (*obj)(j,i)=tmp(j,dims[0]-i-1);
	}
      }
      args.GetReturnValue().Set(args.Holder());  
    }

    static void swapy(const v8::FunctionCallbackInfo<v8::Value>& args) {
      //Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      mat<T> tmp(*obj);
      int* dims=obj->dims;
      for(int j=0;j<dims[1];j++){
	for(int i=0;i<dims[0];i++){
	  (*obj)(j,i)=tmp(dims[1]-j-1,i);
	}
      }
      args.GetReturnValue().Set(args.Holder());  
    }


    static void set_data(const v8::FunctionCallbackInfo<v8::Value>& args) {
      
      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      if (args.Length() < 3) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments, excpecting : (Dx, Dy, Buffer)")));
	return;
      }
      
      Handle<Integer> dx = Handle<Integer>::Cast(args[0]);
      Handle<Integer> dy = Handle<Integer>::Cast(args[1]);

      T* buf = (T*) node::Buffer::Data(args[2]);

      int Dx=dx->NumberValue();
      int Dy=dy->NumberValue();

      MINFO << "Creating image " << Dx << " X " << Dy << endl;
      obj->redim(Dx,Dy);
	    
      memcpy(obj->c, buf,obj->dim*sizeof(T)); 
      
      
    }
    
    static void get_data(const v8::FunctionCallbackInfo<v8::Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      MINFO << "DataCheck " << obj->c[0] << ", " << obj->c[obj->dim-1] << endl;
      MaybeLocal<v8::Object> buu =node::Buffer::Copy(isolate, (const char*) obj->c, obj->dim*sizeof(T));

      Local<Object> cbuu = buu.ToLocalChecked();
      
      //float * b=(float*)node::Buffer::Data(cbuu);
      //MINFO << "DataCheck after " << b[0] << ", " << b[obj->dim-1] << endl;
      
      args.GetReturnValue().Set(cbuu);
      
      // MaybeLocal<Object> slowBuffer = node::Buffer::New(isolate, obj->dim*4);
      
      // Buffer:Data gives us a yummy void* pointer to play with to our hearts
      // content.
      // Local<Object> checked_buf=slowBuffer.ToLocalChecked();
	
      // float* b=(float*)node::Buffer::Data(checked_buf);
      
      // for(int i=0;i<obj->dim;i++){
      // 	float v=(float) obj->c[i];
      // 	memcpy(b+i, &v, 4);
      // }
      
      //args.GetReturnValue().Set(checked_buf);
      
      
      // // Now we need to create the JS version of the Buffer I was telling you about.
      // // To do that we need to actually pull it from the execution context.
      // // First step is to get a handle to the global object.
      //v8::Local<v8::Object> globalObj = isolate->GetCurrentContext()->Global();//v8::Context::GetCurrent()->Global();

      // // Now we need to grab the Buffer constructor function.
      //v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::NewFromUtf8(isolate,"Buffer")));

      // // Great. We can use this constructor function to allocate new Buffers.
      // // Let's do that now. First we need to provide the correct arguments.
      // // First argument is the JS object Handle for the SlowBuffer.
      // // Second arg is the length of the SlowBuffer.
      // // Third arg is the offset in the SlowBuffer we want the .. "Fast"Buffer to start at.
      //v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer, v8::Integer::New(isolate, obj->dim*4), v8::Integer::New(isolate, 0) };

      // // Now we have our constructor, and our constructor args. Let's create the 
      // // damn Buffer already!
      //v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

      // // This Buffer can now be provided to the calling JS code as easy as this:
      //args.GetReturnValue().Set(actualBuffer);

    }
    
    
    static void gen_histogram(const v8::FunctionCallbackInfo<v8::Value>& args) {

      Isolate* isolate = args.GetIsolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(args.Holder());

      
      jsmat<T>& imgdata = *obj;
      
      if (args.Length() < 2) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong number of arguments")));
	return;
      }
      if (!args[0]->IsObject()) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"First argument must be an Object")));
	return;
      }
      if (!args[1]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Second argument must be a callback function")));
	return;
      }

      Handle<Object> ar = args[0]->ToObject();
      Handle<Function> result_cb=Handle<Function>::Cast(args[1]);
      
      Handle<Array> cuts_array = Handle<Array>::Cast(ar->Get(String::NewFromUtf8(isolate,"cuts")));
      
      double cuts[2];
      
      if(cuts_array->IsUndefined()) {
	cuts[0]=imgdata.min();
	cuts[1]=imgdata.max();
      }else{
	cuts[0]= cuts_array->Get(0)->NumberValue();
	cuts[1]= cuts_array->Get(1)->NumberValue();

      }

      Handle<Number> nbins_value = Handle<Number>::Cast(ar->Get(String::NewFromUtf8(isolate,"nbins")));
      
      int nbins=nbins_value->IsUndefined() ? 200 :  nbins_value->Value();
      
      //MINFO << "Cuts " << cuts[0] << ", " << cuts[1] << " nbins = " << nbins << endl;
      //      args[0]->NumberValue();
      double low= cuts[0];
      double max= cuts[1];
      double bsize=(max-low)/nbins;

      v8::Handle<v8::Array> histo_data = v8::Array::New(isolate);
      v8::Handle<v8::Object> result = v8::Object::New(isolate);
      v8::Handle<v8::Number> start = v8::Number::New(isolate,low+.5*bsize);
      v8::Handle<v8::Number> step = v8::Number::New(isolate,bsize);

      result->Set(String::NewFromUtf8(isolate,"start"), start);
      result->Set(String::NewFromUtf8(isolate,"step"), step);
      result->Set(String::NewFromUtf8(isolate,"data"), histo_data);

      /*      
      if(bsize<1){
	bsize=1.0;
	nbins=max-low;
      }
      */
 
      //Handle<String> histo_csv_data = obj->create_image_histogram(cuts);
      //  cout << " HISTO OK : " << *(String::AsciiValue(histo_csv_data->ToString())) << endl;
      try{
	
	vec<unsigned int> histo(nbins);
	histo.set_all(0);
	
	for(int d=0;d<imgdata.dim;d++) 
	  if(imgdata[d]>=low&&imgdata[d]<max){
	    int bid=(int)( (imgdata[d]-low)/bsize);
	    if(bid>=0&&bid<histo.dim)
	      histo[bid]++; 
	  }
	
	//cout << "OK3" << endl;
	
	//	result_string="pixvalue\tndata\n";
	
	//char sb[256];
	
	for(int i=0;i<nbins;i++){
	  histo_data->Set(v8::Number::New(isolate,i),Number::New(isolate,histo[i]));

	  //sprintf(sb,"%g\t%g\n",low+(i+.5)*bsize, histo[i]*1.0);
	  //  result_string+=sb;
	}
	
	
	//	Handle<String> result = v8::String::NewFromUtf8(isolate,result_string.c_str());
	
	//cout << "OK5" << endl;
	//	return result;
      }
      
      catch (qk::exception& e){
	//cout << "Catch qk exception " << endl;
	isolate->ThrowException(v8::String::NewFromUtf8(isolate,e.mess.c_str()));
	return;
      }
      
      const unsigned argc = 2;
      Handle<Value> argv[argc] = { Undefined(isolate), result };
      result_cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
      
      args.GetReturnValue().Set(result);
      
    }
    
  };
  
  // template <class T> inline jsmat<T>* jsmat_unwrap(v8::Handle<v8::Object> handle) {
    
    
  //   assert(!handle.IsEmpty());
  //   assert(handle->InternalFieldCount() > 0);
  //   // Cast to ObjectWrap before casting to T.  A direct cast from void
  //   // to T won't work right when T has more than one base class.
  //   void* ptr = handle->GetPointerFromInternalField(0);
  //   ObjectWrap* wrap = static_cast<ObjectWrap*>(ptr);
    
  //   return static_cast<jsmat<T> *>(wrap);
  // }


  template <typename T> Persistent<Function> jsmat<T>::constructor;
}

    
#endif
    
