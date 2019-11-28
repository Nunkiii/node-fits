/*
- 2003 - 2014 - The Qk/Sadira project - P.G. Sprimont @ INAF, Bologna, IT. 
*/

#ifndef __FITS_HH__
#define __FITS_HH__

#include <string>
#include <fitsio.h>

//#include <nan.h>

#include <math/jsvec_nan.hh>
#include <math/jsmat_nan.hh>


namespace sadira{
  
  using namespace std;
  using namespace v8;
  using namespace Nan;
  using namespace qk;
  
  class fits : public ObjectWrap {

  public:
    
    static NAN_MODULE_INIT(Init) {
      Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
      
      
      tpl->InstanceTemplate()->SetInternalFieldCount(9);
      tpl->SetClassName(Nan::New<v8::String>("fits").ToLocalChecked());
      
      SetPrototypeMethod(tpl, "open", open);
      SetPrototypeMethod(tpl, "get_headers", get_headers);
      SetPrototypeMethod(tpl, "get_headers_array", get_headers_array);
      SetPrototypeMethod(tpl, "set_hdu",set_hdu);
      SetPrototypeMethod(tpl, "set_file",set_file);
      SetPrototypeMethod(tpl, "set_header_key",set_header_key);
      SetPrototypeMethod(tpl, "get_table_column",get_table_column);
      SetPrototypeMethod(tpl, "get_table_columns",get_table_columns);
      SetPrototypeMethod(tpl, "get_table_data",get_table_data);
      SetPrototypeMethod(tpl, "write_image_hdu",write_image_hdu);
      SetPrototypeMethod(tpl, "read_image_hdu",read_image_hdu);


      v8::Isolate* isolate=target->GetIsolate();
      target->Set(v8::String::NewFromUtf8(isolate,"file"), tpl->GetFunction());
      //target->Set(Nan::New<v8::String>("file").ToLocalChecked, tpl->GetFunction());
      
      constructor().Reset(GetFunction(tpl).ToLocalChecked());

    }

    // static void NewInstance(const FunctionCallbackInfo<Value>& args){
      
    //   Isolate* isolate = args.GetIsolate();

    //   Local<Context> context = isolate->GetCurrentContext();

    //   if (args.IsConstructCall()) {
    // 	// Invoked as constructor: `new MyObject(...)`
    // 	//double value = args[0]->IsUndefined() ?
    // 	//  0 : args[0]->NumberValue(context).FromMaybe(0);
    // 	fits* obj = new fits(); //value);
    // 	obj->Wrap(args.This());
    // 	args.GetReturnValue().Set(args.This());
    //   } else {
    // 	// Invoked as plain function `MyObject(...)`, turn into construct call.
    // 	const int argc = 1;
    // 	Local<Value> argv[argc] = { args[0] };
    // 	Local<Function> cons = Local<Function>::New(isolate, constructor);
    // 	Local<Object> result =
    // 	  cons->NewInstance(context, argc, argv).ToLocalChecked();
    // 	args.GetReturnValue().Set(result);
    //   }
      
    //   // const unsigned argc = 1;
    //   // Local<Value> argv[argc] = { args[0] };
    //   // Local<Function> cons = Local<Function>::New(isolate, constructor);


    //   // //      Local<Function> cons = Nan::New<Function>(constructor);
    //   // //args.GetReturnValue().Set(cons->NewInstance(argc, argv));

    //   // Local<Object> instance = cons->NewInstance(argc, argv);
      
    //   // args.GetReturnValue().Set(instance);
    // }

  private:
    
    explicit fits();
    ~fits();
    
    void open_file(const char* file_name, int mode=0);
    void close_file();
    Local<Object> get_headers_array(Isolate* isolate);

    void write_key_str(const string& _keyword, const string& _key_val, const string& _comment);
    void read_key_str(const string& _key_name,string& _key_val);
    
    int get_num_hdu();
    void set_current_hdu(int _hdu_id);
    int get_current_hdu();
    int get_hdu_type();
    void get_img_hdu_size(mem<long>&hdims);

    void send_status_message(Isolate* isolate, Local<Function>& cb,const string& type, const string& message);

    void check_file_is_open(const Nan::FunctionCallbackInfo<Value>& args, int mode=0);

    //    Local<node::Buffer> gen_pngtile(Local<Array>& parameters);
    
    static void New(const Nan::FunctionCallbackInfo<Value>& args);
    static void open(const Nan::FunctionCallbackInfo<Value>& args);
    static void set_file(const Nan::FunctionCallbackInfo<Value>& args);
    static void get_headers(const Nan::FunctionCallbackInfo<Value>& args);
    static void get_headers_array(const Nan::FunctionCallbackInfo<Value>& args);
    static void set_header_key(const Nan::FunctionCallbackInfo<Value>& args);
    static void write_image_hdu(const Nan::FunctionCallbackInfo<Value>& args);
    static void read_image_hdu(const Nan::FunctionCallbackInfo<Value>& args);
    static void set_hdu(const Nan::FunctionCallbackInfo<Value>& args);
    static void get_table_column(const Nan::FunctionCallbackInfo<Value>& args);
    static void get_table_columns(const Nan::FunctionCallbackInfo<Value>& args);
    static void get_table_data(const Nan::FunctionCallbackInfo<Value>& args);

    
    Local<Object> get_table_column(Isolate* isolate, int column_id);
    Local<Object> get_table_columns(Isolate* isolate);
    Local<Object> get_table_columns_hash(Isolate* isolate);
    Local<Array> get_table_data(Isolate* isolate, int,int);

    void report_fits_error();

    //    Local<String> create_image_histogram(double* cuts);
    //    static Local<Value> gen_histogram(const Nan::FunctionCallbackInfo<Value>& args);

    double counter_;

    string get_file_name(const Nan::FunctionCallbackInfo<Value>& args);
    
    template <typename T>

    void write_key(T& _numobj, const string& _comment=""){
      int ft=get_fits_type<T>();
      ffpky(f, ft,  _numobj.get_name().c_str(), &_numobj, _comment.c_str(), &fstat);
      report_fits_error();
    }
    
    
    void write_key( Isolate* isolate, Local<Object> opts);
    
    template <typename T>
    void write_key(const string& _keyword, T _key_val, const string& _comment=""){
      int ft=get_fits_type<T>();
      ffpky(f, ft, _keyword.c_str(), &_key_val, _comment.c_str(), &fstat);
      report_fits_error();
    }




    template <typename T> 
    void read_key(const string& _key_name,T& _key_val){
      int dt=get_fits_type<T>();
      fits_read_key(f,dt,(char*)_key_name.c_str(),&_key_val,NULL,&fstat);
      report_fits_error();
    }

    
    template <typename T> void write_image(mat<T>& _m){
      
      fstat=0;
      
      long ndims[2];
      long fpix[2]={1,1};

      int fits_image_type=get_fits_image_type<T>();
      int fits_type=get_fits_type<T>();

      ndims[0]=_m.dims[0];
      ndims[1]=_m.dims[1];

      MINFO << "write_image: Image Type="<< fits_image_type << ", Fits Type "<< fits_type << endl;
      
      fits_create_img(f, fits_image_type, 2, ndims, &fstat);
      report_fits_error();

      mat<T> check; check.redim(_m.dims[0],_m.dims[1]);
      
      // for(int k=0;k<20;k++){
      // 	cout << "check data before ["<<k<<"] : " << _m[k] << endl;
      // }
      
      fits_write_subset(f, fits_type, fpix, ndims, _m.c, &fstat);

      report_fits_error();
      long nel=check.dim;
      int anynul=0;
      
      fits_read_img(f, fits_type, 1, nel, NULL, check.data_pointer(), &anynul, &fstat);
      //fits_read_subset (f, fits_type, fpix, NULL, NULL,	NULL, check.c , NULL, &fstat);

      // for(int k=0;k<20;k++){
      // 	cout << "check data ["<<k<<"] : " << check[k] << endl;
      // }
      
    }

  private:
    
    static inline Nan::Persistent<v8::Function> & constructor() {
      static Nan::Persistent<v8::Function> my_constructor;
      return my_constructor;
    }
    
    
  protected:
    


    
    template <typename T> int get_fits_type(){
      
      if (typeid(T)==typeid(unsigned char)) return TBYTE;
      if (typeid(T)==typeid(char)) return TSBYTE;
      if (typeid(T)==typeid(short int)) return TSHORT;
      if (typeid(T)==typeid(unsigned short int)) return TUSHORT;
      if (typeid(T)==typeid(int)) return TINT;
      if (typeid(T)==typeid(long int)) return TLONGLONG;
      if (typeid(T)==typeid(unsigned int)) return TULONG;
      if (typeid(T)==typeid(double)) return TDOUBLE;
      if (typeid(T)==typeid(float)) return TFLOAT;
      if (typeid(T)==typeid(string)) return TSTRING;
      
      
      throw qk::exception("Impossible to convert requested type to a valid FITS type" );
    }
    
    int get_equiv_image_type(int _img_ftype);

    template <typename T> int get_fits_image_type(){

// #define BYTE_IMG      8  /* BITPIX code values for FITS image types */
// #define SHORT_IMG    16
// #define LONG_IMG     32
// #define LONGLONG_IMG 64
// #define FLOAT_IMG   -32
// #define DOUBLE_IMG  -64
//                          /* The following 2 codes are not true FITS         */
//                          /* datatypes; these codes are only used internally */
//                          /* within cfitsio to make it easier for users      */
//                          /* to deal with unsigned integers.                 */
// #define SBYTE_IMG    10
// #define USHORT_IMG   20
// #define ULONG_IMG    40


      
      if (typeid(T)==typeid(unsigned char)) return BYTE_IMG;
      if (typeid(T)==typeid(char)) return SBYTE_IMG;
      if (typeid(T)==typeid(short int)) return SHORT_IMG;
      if (typeid(T)==typeid(unsigned short int)) return USHORT_IMG;
      if (typeid(T)==typeid(int)) return LONG_IMG;
      if (typeid(T)==typeid(unsigned int)) return ULONG_IMG;
      if (typeid(T)==typeid(long int)) return LONGLONG_IMG;
      if (typeid(T)==typeid(float)) return FLOAT_IMG;
      if (typeid(T)==typeid(double)){ MINFO << "OK double ! " << endl; return DOUBLE_IMG;}
      

      throw qk::exception("Impossible to convert to valid FITS image type for cpp typeid = " + string(typeid(T).name()) );
    }
    
    template <typename T> void read_image(mat<T>& _img){
      mem<long> img_hdu_dims;
      get_img_hdu_size(img_hdu_dims);
      vec<long> fpix(img_hdu_dims.dim);
      fpix.set_all(1);

      _img.redim(img_hdu_dims[0],img_hdu_dims[1]);

      long ntoread=img_hdu_dims[0]*img_hdu_dims[1];
      int ftype=get_fits_type<T>();

      fits_read_pix(f,ftype,fpix.c,ntoread,0,_img.c,0,&fstat); report_fits_error();
    }

    
    Local<Value> create_matrix_type(Isolate* isolate, int _ftype, mem<long>&hdd){

      const int argc = 2;
      Local<Value> argv[argc] = { Nan::New((int32_t) hdd[0]),  Nan::New((int32_t) hdd[1]) };
      v8::Local<v8::Function> cons;
      //Local<Nan::Persistent<v8::Function>> cons; 
      //Local<Object> instance = cons->NewInstance(argc, argv);
      
      //Local<Object> instance = jsmat<unsigned char>::NewInstance();
      
      switch (_ftype){
      case TBYTE: cons = Nan::New<v8::Function>( jsmat<unsigned char>::constructor()); break;
      case TSBYTE: cons = Nan::New<v8::Function>( jsmat<char>::constructor()); break;
      case TSHORT: cons = Nan::New<v8::Function>( jsmat<short int>::constructor()); break;
      case TUSHORT: cons = Nan::New<v8::Function>( jsmat<unsigned short int>::constructor()); break;
      case TLONG: cons = Nan::New<v8::Function>( jsmat<int>::constructor()); break;
      case TULONG: cons = Nan::New<v8::Function>( jsmat<unsigned int>::constructor()); break;
      case TLONGLONG: cons = Nan::New<v8::Function>( jsmat<long int>::constructor());break;
      case TDOUBLE: cons = Nan::New<v8::Function>( jsmat<double>::constructor());break;
      case TFLOAT: cons = Nan::New<v8::Function>( jsmat<float>::constructor());break;
      default:
	MERROR << "Cannot find suitable jsmat type for FITS data type " << _ftype << endl;
	return Local<Value>(Undefined(isolate));
      };


      //Local<Context> context = isolate->GetCurrentContext();
      Local<Object> result =Nan::NewInstance(cons,  argc, argv).ToLocalChecked();
      
      return result; //cons->NewInstance(argc, argv); 
    }

    
    //std::string file_name;
    char fitsio_error_buffer[1024];
    FILE* ffitsio_error;
    int fstat;
    int c_hdu;
    
    fitsfile* f;
  };

      
  
  
  
}

#endif
