#ifndef __JSMAT_HH__
#define __JSMAT_HH__

#include <qk/mat.hh>
#include <math/jsvec_nan.hh>
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
  //  using namespace v8;
  using namespace Nan;
  
  void free_stream_buffer(char* b, void*x);

  void callback(char* data, void* hint) {
    free(data);
  }
  
  
  
  template <typename T> class jsmat 
    : public ObjectWrap, public mat<T> {
    
  public:

    //static Nan::Persistent<Function> constructor;
    static void init(v8::Local<v8::Object> exports, const char* class_name){
      //    static NAN_MODULE_INIT(Init) {
      //Isolate* isolate = exports->Get//Isolate();
      
      v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
      
      tpl->SetClassName(Nan::New(class_name).ToLocalChecked());
      
      tpl->InstanceTemplate()->SetInternalFieldCount(20);
      
      SetPrototypeMethod(tpl, "length",length);
      SetPrototypeMethod(tpl, "width",width);
      SetPrototypeMethod(tpl, "height",height);
      SetPrototypeMethod(tpl, "resize",resize);
      SetPrototypeMethod(tpl, "crop",crop);
      SetPrototypeMethod(tpl, "copy",copy);
      SetPrototypeMethod(tpl, "extend",extend);
      SetPrototypeMethod(tpl, "swapx",swapx);
      SetPrototypeMethod(tpl, "swapy",swapy);
      SetPrototypeMethod(tpl, "set_colormap", set_colormap);
      SetPrototypeMethod(tpl, "set_cuts", set_cuts);
      SetPrototypeMethod(tpl, "set_cuts_histo", set_cuts_histo);
      SetPrototypeMethod(tpl, "histogram", gen_histogram);
      SetPrototypeMethod(tpl, "get_data", get_data);
      SetPrototypeMethod(tpl, "set_data", set_data);
      SetPrototypeMethod(tpl, "set_all", jset_all);
      SetPrototypeMethod(tpl, "tile",tile);
      SetPrototypeMethod(tpl, "add",add);

      SetPrototypeMethod(tpl, "get_value", jsvec<T>::get_value);
      SetPrototypeMethod(tpl, "set_value", jsvec<T>::set_value);
      
      constructor().Reset(GetFunction(tpl).ToLocalChecked());
      //exports->Set(String::NewFromUtf8(isolate, class_name),tpl->GetFunction());
    }
    
    qk::colormap<float> cmap;
    qk::colormap_array<float> scaled_cmap;
    double cuts[2];
    
  private:

    static NAN_METHOD(New) {
      
      //Isolate* isolate = info.Get//Isolate();
      //v8::Local<Context> context = isolate->GetCurrentContext();
      
      if (info.IsConstructCall()) {
	// Invoked as constructor: `new MyObject(...)`
	int d0= (int) info[0]->IsUndefined() ? 0 : info[0]->NumberValue();
	int d1= (int) info[1]->IsUndefined() ? 0 : info[1]->NumberValue();
	jsmat* obj = new jsmat(d0,d1);
	obj->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
      } else {
	// Invoked as plain function `MyObject(...)`, turn into construct call.
	const int argc = 2;
	v8::Local<v8::Value> argv[argc] = { info[0], info[1] };
	v8::Local<v8::Function> cons = Nan::New(constructor());
	info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked()); //cons->NewInstance(argc, argv));
      }
      
    }

  public:
    explicit jsmat (int _d0=0, int _d1=0)  
      :mat<T>(_d0,_d1){
      //cout << "New matrix "<< this <<" D="<<dims[0]<<"," << dims[1] << " D="<< dim << " datap="<< ((void*)data_pointer()) << endl;
    }
    jsmat (const jsmat<T> & m):ObjectWrap(),mat<T>(m){
      
    }   

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

    static NAN_METHOD(NewInstance) {
      v8::Local<v8::Function> cons = Nan::New(constructor());
      //double value = info[0]->IsNumber() ? To<double>(info[0]).FromJust() : 0;
      const int argc = 2;
      v8::Local<v8::Value> argv[argc] = {info[0],info[1]};
      info.GetReturnValue().Set(
				Nan::NewInstance(cons, argc, argv).ToLocalChecked());
      
      //Isolate* isolate = info.Get//Isolate();
      //v8::Local<Context> context = isolate->GetCurrentContext();
      
      // const unsigned argc = 2;
      // v8::Local<v8::Value> argv[argc] = { info[0], info[1] };
      // v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, constructor);
      // v8::Local<v8::Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

      // info.GetReturnValue().Set(instance);
    }

    
    static void length(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      info.GetReturnValue().Set(Nan::New(obj->dim));
    }

    static void set_cuts_histo(const Nan::FunctionCallbackInfo<v8::Value>& info){}
    
    static void set_cuts(const Nan::FunctionCallbackInfo<v8::Value>& info){

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());
      
      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New("Wrong number of arguments").ToLocalChecked());
	return;
      }

      if(! info[0]->IsArray()){
	Nan::ThrowError(Nan::New( "Argument must be an Array").ToLocalChecked());
	return;
      }
      v8::Handle<v8::Array> cutsa = v8::Handle<v8::Array>::Cast(info[0]);
      
      obj->cuts[0]=cutsa->Get(0)->NumberValue();
      obj->cuts[1]=cutsa->Get(1)->NumberValue();
      obj->rescale_colormap();

      info.GetReturnValue().Set(info.Holder());

    }

    static void add(const Nan::FunctionCallbackInfo<v8::Value>& info){
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      
      v8::Local<v8::Object> ar=v8::Local<v8::Object>::Cast(info[0]);
      
      jsmat* m = ObjectWrap::Unwrap<jsmat>(ar);

      (*obj)+=(*m);
      
      info.GetReturnValue().Set(info.Holder());
    }

    static void jset_all(const Nan::FunctionCallbackInfo<v8::Value>& info){
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      
      T value = (T) (info[0]->IsUndefined() ? 0 : info[0]->NumberValue());
      
      
      obj->set_all(value);
      
      info.GetReturnValue().Set(info.Holder());
    }

    
  
    void rescale_colormap(){

      scaled_cmap = cmap;
      
      int ci;

      for(ci=0;ci<scaled_cmap.dim;ci++){
	scaled_cmap[ci][4]*=(cuts[1]-cuts[0]);
	scaled_cmap[ci][4]+=cuts[0];
      }
      
    }
    
    
    static void set_colormap(const Nan::FunctionCallbackInfo<v8::Value>& info){

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }

      if(! info[0]->IsArray()){
	Nan::ThrowError(Nan::New( "Argument must be an Array").ToLocalChecked());
	return;
      }

      v8::Handle<v8::Array> cmap_data = v8::Handle<v8::Array>::Cast(info[0]);
      
      obj->cmap.ttd();
      qk::colormap_value<float>* cmv;
      unsigned int ncolors=cmap_data->Length();
      
      for(unsigned int c=0;c<ncolors;c++){
	cmv=new qk::colormap_value<float>();
	//  MINFO << "Reading color "<<c << endl;
	v8::Local<v8::Array> cmva = v8::Local<v8::Array>::Cast(cmap_data->Get(c));
	for(int cpn=0;cpn<5;++cpn){
	  //  MINFO << "read " << cmap[c][cpn].asFloat() << endl;
	  (*cmv)[cpn]=(float)cmva->Get(cpn)->NumberValue();
	}
	
	obj->cmap.add(cmv);
      }  
      info.GetReturnValue().Set(info.Holder());
    }
    
    static void resize(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 2) {
	Nan::ThrowError(Nan::New("You must pass width and height as parameters").ToLocalChecked());
	return;
      }
      if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
	Nan::ThrowError(Nan::New("Arguments must be numbers").ToLocalChecked());
	return;
      }
      
      obj->redim(info[0]->NumberValue(), info[1]->NumberValue());
      info.GetReturnValue().Set(info.Holder());
    }

    static void width(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());
      v8::Local<v8::Number> d= Nan::New(obj->dims[0]);
      info.GetReturnValue().Set(d);
    }

    static void height(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());
      v8::Local<v8::Number> d=Nan::New(obj->dims[1]);
      info.GetReturnValue().Set(d);
    }


    static void copy(const Nan::FunctionCallbackInfo<v8::Value>& info) {}

    static void crop(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      if (!info[0]->IsObject()) {
	Nan::ThrowError(Nan::New("Argument must be an object").ToLocalChecked());
	return;
      }

      v8::Local<v8::Object> params = v8::Local<v8::Object>::Cast(info[0]->ToObject()); //->Get(String::NewFromUtf8(isolate,"cuts")));
      
      v8::Handle<v8::Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(Nan::New("x").ToLocalChecked());
      if(!hv->IsUndefined()) r[0]=hv->NumberValue();
      
      hv=params->Get(Nan::New("y").ToLocalChecked()); if(hv != Undefined()) r[1]=hv->NumberValue();
      hv=params->Get(Nan::New("w").ToLocalChecked()); if(hv != Undefined()) r[2]=hv->NumberValue();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(Nan::New("h").ToLocalChecked()); if(hv != Undefined()) r[3]=hv->NumberValue();else r[3]=obj->dims[1]-r[1];

      //cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;
      
      if(obj->is_in(r)){
	//v8::String::Utf8Value fffu(hv->ToString());
	cout << "OK rect is inside !"<< endl;
	if(r[2]==obj->dims[0]&&r[3]==obj->dims[1]){
	  info.GetReturnValue().Set(info.Holder());
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
	Nan::ThrowError(Nan::New("Invalid area for cropping").ToLocalChecked());
	return;
      }
      info.GetReturnValue().Set(info.Holder());
    }


    static void extend(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      //cout << "Extend !" << endl;
      if (!info[0]->IsObject()) {
	Nan::ThrowError(Nan::New("Argument must be an object").ToLocalChecked());
	return;
      }

      v8::Local<v8::Object> params = v8::Local<v8::Object>::Cast(info[0]->ToObject()); //->Get(Nan::New(*Utf8String("cuts")).ToLocalChecked());

      v8::Handle<v8::Value> hv; 
      rect<int> r(0,0,-1,-1);
      
      hv=params->Get(Nan::New("x").ToLocalChecked()); if(hv != Undefined()) r[0]=hv->NumberValue();
      hv=params->Get(Nan::New("y").ToLocalChecked()); if(hv != Undefined()) r[1]=hv->NumberValue();
      hv=params->Get(Nan::New("w").ToLocalChecked()); if(hv != Undefined()) r[2]=hv->NumberValue();else r[2]=obj->dims[0]-r[0];
      hv=params->Get(Nan::New("h").ToLocalChecked()); if(hv != Undefined()) r[3]=hv->NumberValue();else r[3]=obj->dims[1]-r[1];

      //cout << "rect is " << r[0] << ", "<< r[1] << ", "<< r[2] << ", "<< r[3] << endl;

      if(r[2]<=obj->dims[0] || r[3]<=obj->dims[1] ){
	Nan::ThrowError(Nan::New("Invalid area for cropping").ToLocalChecked());
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

      info.GetReturnValue().Set(info.Holder());
    }

    static void tile(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());
      
      if (info.Length() < 1) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      
      if (!info[0]->IsObject()) {
	Nan::ThrowError(Nan::New("Argument must be an Object").ToLocalChecked());
	return;
     }
      
      v8::Handle<v8::Array> parameters = v8::Handle<v8::Array>::Cast(info[0]);

      v8::Handle<v8::Array> tile_coord = v8::Handle<v8::Array>::Cast(parameters->Get(Nan::New("tile_coord").ToLocalChecked()));
      v8::Handle<v8::Array> tile_size = v8::Handle<v8::Array>::Cast(parameters->Get(Nan::New("tile_size").ToLocalChecked()));
      v8::Handle<v8::Integer> zoom_par = v8::Handle<v8::Integer>::Cast(parameters->Get(Nan::New("zoom").ToLocalChecked()));
      v8::Handle<v8::String> type_par= v8::Handle<v8::String>::Cast(parameters->Get(Nan::New("type").ToLocalChecked()));

      int x_tile,y_tile,zoom, ts[2];
      
      if(tile_coord->IsUndefined()) {
	x_tile=0;
	y_tile=0;
      }else{
	x_tile= v8::Handle<v8::Integer>::Cast(tile_coord->Get(0))->Value();
	y_tile= v8::Handle<v8::Integer>::Cast(tile_coord->Get(1))->Value();
      }

      if(tile_size->IsUndefined()) {
	ts[0]=ts[1]=256;
      }else{
	ts[0]=v8::Handle<v8::Integer>::Cast(tile_size->Get(0))->Value();
	ts[1]=v8::Handle<v8::Integer>::Cast(tile_size->Get(1))->Value();
      }
      
      if(zoom_par->IsUndefined()) {
	zoom=0;
      }else
	zoom=v8::Handle<v8::Integer>::Cast(zoom_par)->Value();

      string type ="jpeg";
      if(! type_par->IsUndefined()){
      	v8::String::Utf8Value s(type_par->ToString());
	type=*s;
      }

      //MINFO << "Building tile sz " << ts[0] << ", " << ts[1] << " coord " << x_tile << ", " << y_tile << " z= " << zoom << " type " << type << endl;

      //return scope.Close(info.This());
      
      try{
	//v8::Handle<node::Buffer> bp = obj->gen_pngtile(parameters, tile_size);
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
	
	//	MaybeLocal<v8::Object> mbp=
	//v8::Local<v8::Object> bp=mbp.ToLocalChecked();
	
	//memcpy(Buffer::Data(bp), stream_data, stream_size);
	
	//v8::Handle<Buffer> hbp(bp);
	
	info.GetReturnValue().Set(Nan::NewBuffer(stream_data, stream_size, callback, 0).ToLocalChecked());  
      }
      
      catch (qk::exception& e){
	//    MERROR << "Failed: "<<e.mess << endl;
	Nan::ThrowError(Nan::New(e.mess.c_str()).ToLocalChecked());
      }
    }

    static void swapx(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      ////Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      mat<T> tmp(*obj);
      int* dims=obj->dims;
      for(int j=0;j<dims[1];j++){
	for(int i=0;i<dims[0];i++){
	  (*obj)(j,i)=tmp(j,dims[0]-i-1);
	}
      }
      info.GetReturnValue().Set(info.Holder());  
    }

    static void swapy(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      ////Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      mat<T> tmp(*obj);
      int* dims=obj->dims;
      for(int j=0;j<dims[1];j++){
	for(int i=0;i<dims[0];i++){
	  (*obj)(j,i)=tmp(dims[1]-j-1,i);
	}
      }
      info.GetReturnValue().Set(info.Holder());  
    }


    static void set_data(const Nan::FunctionCallbackInfo<v8::Value>& info) {
      
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      if (info.Length() < 3) {
	Nan::ThrowError(Nan::New("Wrong number of arguments, excpecting : (Dx, Dy, Buffer)").ToLocalChecked());
	return;
      }
      
      v8::Handle<v8::Integer> dx = v8::Handle<v8::Integer>::Cast(info[0]);
      v8::Handle<v8::Integer> dy = v8::Handle<v8::Integer>::Cast(info[1]);

      T* buf = (T*) node::Buffer::Data(info[2]);

      int Dx=dx->NumberValue();
      int Dy=dy->NumberValue();

      MINFO << "Creating image " << Dx << " X " << Dy << endl;
      obj->redim(Dx,Dy);
	    
      memcpy(obj->c, buf,obj->dim*sizeof(T)); 
      
      
    }
    
    static void get_data(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      MINFO << "GET DATA" << endl;
      
      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());
      if(obj->dim>1)
	MINFO << "DataCheck 0,1 : " << obj->c[0] << ", " << obj->c[obj->dim-1] << endl;
      else
	MINFO << "DataCheck : Empty!" << endl;
      
      
      
      
      
      //const char* dataptr=(const char*) obj->c;

      //Buffer buu=node::Buffer::New(isolate, (char*) obj->c, obj->dim*sizeof(T));

      //MaybeLocal<v8::Object> buu =
      
      //MaybeLocal<v8::Object> buu = node::Buffer::New(isolate, (char*) obj->c, obj->dim*sizeof(T));
      //MaybeLocal<v8::Object> buu = node::Buffer::New(isolate, obj->dim*sizeof(T));
      
      //v8::Local<v8::Object> cbuu = buu.ToLocalChecked();

      //char * dat = node::Buffer::Data(cbuu);
      //T* datu = (T*) dat;
      
      //MINFO << "POST DataCheck 0,1 : " << datu[0] << ", " << datu[1] << endl;
      
      info.GetReturnValue().Set(Nan::CopyBuffer( (const char*) obj->c, obj->dim*sizeof(T)).ToLocalChecked());


      //      info.GetReturnValue().Set(node::Buffer::New(isolate, (char*) obj->c, obj->dim*sizeof(T)).ToLocalChecked());
      
      //T * b=(T*)node::Buffer::Data(cbuu);
      //MINFO << "DataCheck after " << b[0] << ", " << b[obj->dim-1] << endl;
      
      

      
      
      // MaybeLocal<Object> slowBuffer = node::Buffer::New(isolate, obj->dim*4);
      
      // Buffer:Data gives us a yummy void* pointer to play with to our hearts
      // content.
      // Local<Object> checked_buf=slowBuffer.ToLocalChecked();
	
      // float* b=(float*)node::Buffer::Data(checked_buf);
      
      // for(int i=0;i<obj->dim;i++){
      // 	float v=(float) obj->c[i];
      // 	memcpy(b+i, &v, 4);
      // }
      
      //info.GetReturnValue().Set(checked_buf);
      
      
      // // Now we need to create the JS version of the Buffer I was telling you about.
      // // To do that we need to actually pull it from the execution context.
      // // First step is to get a handle to the global object.
      //v8::Local<v8::Object> globalObj = isolate->GetCurrentContext()->Global();//v8::Context::GetCurrent()->Global();

      // // Now we need to grab the Buffer constructor function.
      //v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::Nan::New(*Utf8String("Buffer")).ToLocalChecked()));

      // // Great. We can use this constructor function to allocate new Buffers.
      // // Let's do that now. First we need to provide the correct arguments.
      // // First argument is the JS object Handle for the SlowBuffer.
      // // Second arg is the length of the SlowBuffer.
      // // Third arg is the offset in the SlowBuffer we want the .. "Fast"Buffer to start at.
      //v8::Handle<v8::Value> constructorInfo[3] = { slowBuffer, v8::v8::Integer::New(isolate, obj->dim*4), v8::v8::Integer::New(isolate, 0) };

      // // Now we have our constructor, and our constructor info. Let's create the 
      // // damn Buffer already!
      //v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorInfo);

      // // This Buffer can now be provided to the calling JS code as easy as this:
      //info.GetReturnValue().Set(actualBuffer);

    }
    
    
    static void gen_histogram(const Nan::FunctionCallbackInfo<v8::Value>& info) {

      //Isolate* isolate = info.Get//Isolate();
      jsmat* obj = ObjectWrap::Unwrap<jsmat>(info.Holder());

      
      jsmat<T>& imgdata = *obj;
      
      if (info.Length() < 2) {
	Nan::ThrowError(Nan::New( "Wrong number of arguments").ToLocalChecked());
	return;
      }
      if (!info[0]->IsObject()) {
	Nan::ThrowError(Nan::New("First argument must be an Object").ToLocalChecked());
	return;
      }
      if (!info[1]->IsFunction()) {	
	Nan::ThrowError(Nan::New("Second argument must be a callback function").ToLocalChecked());
	return;
      }

      v8::Handle<v8::Object> ar = info[0]->ToObject();
      v8::Handle<v8::Function> result_cb=v8::Handle<v8::Function>::Cast(info[1]);
      
      v8::Handle<v8::Array> cuts_array = v8::Handle<v8::Array>::Cast(ar->Get(Nan::New("cuts").ToLocalChecked()));
      
      double cuts[2];
      
      if(cuts_array->IsUndefined()) {
	cuts[0]=imgdata.min();
	cuts[1]=imgdata.max();
      }else{
	cuts[0]= cuts_array->Get(0)->NumberValue();
	cuts[1]= cuts_array->Get(1)->NumberValue();

      }

      v8::Handle<v8::Number> nbins_value = v8::Handle<v8::Number>::Cast(ar->Get(Nan::New("nbins").ToLocalChecked()));
      
      int nbins=nbins_value->IsUndefined() ? 200 :  nbins_value->Value();
      
      //MINFO << "Cuts " << cuts[0] << ", " << cuts[1] << " nbins = " << nbins << endl;
      //      info[0]->NumberValue();
      double low= cuts[0];
      double max= cuts[1];
      double bsize=(max-low)/nbins;

      v8::Handle<v8::Array> histo_data = Nan::New<v8::Array>();//::New(isolate);
      v8::Handle<v8::Object> result = Nan::New<v8::Object>();
      v8::Handle<v8::Number> start = Nan::New<v8::Number>(low+.5*bsize);
      v8::Handle<v8::Number> step = Nan::New<v8::Number>(bsize);

      result->Set(Nan::New<v8::String>("start").ToLocalChecked(), start);
      result->Set(Nan::New<v8::String>("step").ToLocalChecked(), step);
      result->Set(Nan::New<v8::String>("data").ToLocalChecked(), histo_data);

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
	  histo_data->Set(Nan::New(i),Nan::New(histo[i]));
	  
	  //sprintf(sb,"%g\t%g\n",low+(i+.5)*bsize, histo[i]*1.0);
	  //  result_string+=sb;
	}
	
	
	//	Handle<String> result = v8::Nan::New(*Utf8String(result_string.c_str());
	
	//cout << "OK5" << endl;
	//	return result;
      }
      
      catch (qk::exception& e){
	//cout << "Catch qk exception " << endl;
	Nan::ThrowError(Nan::New(e.mess.c_str()).ToLocalChecked());
	return;
      }
      
      const unsigned argc = 2;
      v8::Handle<v8::Value> argv[argc] = { Undefined(), result };
      //result_cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );

      v8::Local<v8::Object> target = Nan::New<v8::Object>();
      result_cb->Call(target, argc, argv );    
      
      info.GetReturnValue().Set(result);
      
    }

  public:
    
    static inline Nan::Persistent<v8::Function> & constructor() {
      static Nan::Persistent<v8::Function> my_constructor;
      return my_constructor;
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


  //template <typename T> Nan::Persistent<v8::Function> jsmat<T>::constructor;
}

    
#endif
    
