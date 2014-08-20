#include <iostream>
#include <unistd.h>

#include "fits.hh"


#ifdef __APPLE__
#ifdef __cplusplus

extern "C" {
  FILE * open_memstream (char **buf, size_t *len);
  FILE *fmemopen(void *buf, size_t size, const char *mode);
}

#endif
#endif


using namespace v8;
using namespace node;
using namespace std;

namespace sadira{

  using namespace qk;

  
  Persistent<FunctionTemplate> fits::s_ctf;
  Persistent<Function> fits::constructor;
  
  
  template <class T> Persistent<FunctionTemplate> jsvec<T>::s_ctm;
  template <class T> Persistent<FunctionTemplate> jsmat<T>::s_ctm;
  template <class T> Persistent<Function> jsmat<T>::constructor;


  void fits::report_fits_error(){
    if(fstat){
      rewind(ffitsio_error);
      fits_report_error(ffitsio_error,fstat);
      fflush(ffitsio_error);
      fstat=0;
      throw qk::exception(fitsio_error_buffer);
    }
  }
  
  fits::fits() {
    //    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    
    //MINFO << "Created FITS object ! " << endl;
  }
  
  fits::~fits() {
    MINFO << "destructor called !" << endl;
    close_file();
    fclose(ffitsio_error);
  }
  
  void fits::open_file(const char* file_name, int _mode){


    if(f!=0){ close_file(); f=0;}
    /*
    if(!file_name){//=="" || file_name=="Undefined"){
      throw qk::exception("No file name set, use set_file command to provide one");
    }
    */
    //MINFO << "Opening File name [" << file_name << "]"<<endl;
    
    fstat=0;
    FILE*fff;
    
    switch (_mode){
    case 0:
      fits_open_file(&f, file_name, READONLY, &fstat);
      break;
    case 1:
      fff=fopen(file_name,"r"); 
      if (fff){
	fclose(fff);
	
	//Checking if file exists and user didn't provide a !, we try to delete the file
	if(file_name[0]!='!'){
	  unlink(file_name);
	  fits_create_file(&f, file_name, &fstat);
	}else
	  fits_open_file(&f, file_name,READWRITE, &fstat);
	
      }
      else
	fits_create_file(&f, file_name, &fstat);
      break;
    case 2:
      fits_open_file(&f, file_name,READWRITE, &fstat);
      break;
    default:
      break;
      
    };
    
    report_fits_error();

    MINFO << "OK FITS opened pointer = " << f << endl;
  }


  
  void fits::close_file() {
    if(f){
      fits_close_file(f, &fstat);
      f=0;
      report_fits_error();
    }
  }
  
  void fits::write_key_str(const string& _keyword, const string& _key_val, const string& _comment){
    //ffpky(f, TSTRING, (char*)_keyword.c_str(), (char*)_key_val.c_str(),(char*) _comment.c_str(), &fstat);
    //MINFO << get_info() << " writing key [" << _keyword << "]"<<endl;
  
    ffuky(f, TSTRING, (char*)_keyword.c_str(), (char*)_key_val.c_str(),(char*) _comment.c_str(), &fstat);
    report_fits_error();
  }
  
  void fits::read_key_str(const string& _key_name,string& _key_val){
    char str_data[80];
    //    MINFO << "Looking for key ["<<_key_name<<"], stat="<<fstat << endl;
    fits_read_key(f,TSTRING,(char*)_key_name.c_str(),str_data,NULL,&fstat);
    //    MINFO << "Looking for key ["<<_key_name<<"], stat="<<fstat << " found ["<<str_data<<"]"<<endl;
    report_fits_error();
    _key_val=str_data;
  }
  
  void fits::set_current_hdu(int _hdu_id){
    //check_file_is_open();
    
    //if(c_hdu!=_hdu_id){
    //fstat=0;
    MINFO << "Switching to hdu " << _hdu_id << endl;
    ffmahd (f, _hdu_id+1, NULL, &fstat);  
    report_fits_error();
    c_hdu=_hdu_id;

    //get_img_hdu_size();
    //}
  }
  
  
  int fits::get_current_hdu(){
    fstat=0;
    int chdu=-1;
    ffghdn (f, &chdu);  
    report_fits_error();
    return chdu;
  }
  
  int fits::get_num_hdu(){
    fstat=0;
    int nhdu;
    ffthdu(f,&nhdu,&fstat);
    report_fits_error();
    return nhdu;
  }
  
  int fits::get_hdu_type(){
    fstat=0;
    int hdut;
    ffghdt(f,&hdut,&fstat);
    report_fits_error();
    return hdut;
  }
  


  void fits::get_img_hdu_size(mem<long>&hdims){
    
    set_current_hdu(c_hdu);
    fstat=0;
    //    MINFO << "Getting hdu dimensions..."<<endl;
    int img_hdu_ndims=0;
    fits_get_img_dim(f, &img_hdu_ndims, &fstat);
    report_fits_error();
    
    if(fstat) return;

    hdims.redim(img_hdu_ndims);

    fits_get_img_size(f, img_hdu_ndims, hdims.c, &fstat);
    report_fits_error();

    MINFO << "Full image HDU size " << hdims[0] << ", " << hdims[1] <<endl;
  }

  int fits::get_equiv_image_type(int _img_ftype){
    int ft;
    switch (_img_ftype){
      
    case BYTE_IMG: ft= TBYTE;break;
    case SBYTE_IMG: ft= TSBYTE;break;
    case SHORT_IMG: ft= TSHORT;break;
    case USHORT_IMG: ft=TUSHORT;break;
    case LONG_IMG: ft=TLONG;break;
    case ULONG_IMG: ft=TULONG;break;
    case LONGLONG_IMG: ft=TLONGLONG;break;
    case DOUBLE_IMG: ft=TDOUBLE;break;
    case FLOAT_IMG: ft=TFLOAT;break;
    default: throw qk::exception("Unknown image type !");
      
    };
    return ft;
  }
  


  Handle<Value> fits::New(const Arguments& args) {
    HandleScope scope;

    if (args.IsConstructCall()) {
      
      MINFO << "Createing new FITS object !" << endl;

      fits* obj = new fits();
      obj->Wrap(args.This());
      //  obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
      obj->fstat=0;
      obj->f=0;
      obj->c_hdu=0;
      obj->ffitsio_error = fmemopen(obj->fitsio_error_buffer,sizeof(obj->fitsio_error_buffer),"w");
      
      if(!args[0]->IsUndefined()){
	//string fn;
	v8::String::Utf8Value s(args[0]->ToString());
	//fn=*s;
	args.This()->Set(String::NewSymbol("file_name"), String::New(*s));
      

	open(args);
      }else
	args.This()->Set(String::NewSymbol("file_name"), String::New(""));
      

      return args.This();
      
      //return scope.Close(args.This());
    }else{
      MINFO << "Plain func construct call !" << endl;
      const int argc = 1;
      Local<Value> argv[argc] = { args[0] };
      return scope.Close(constructor->NewInstance(argc, argv));
    }
  }


  void fits::check_file_is_open(const Arguments& args) {

    if(f) return;

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    try{
      
      Handle<Value> fff=args.This()->Get(String::NewSymbol("file_name"));
            
      v8::String::Utf8Value fn(fff->ToString());
      
      //string fn=obj->get_file_name(args);

      MINFO << "Opening " << *fn << endl;
      obj->open_file(*fn);
    }
    catch (qk::exception& e){
      v8::ThrowException(v8::String::New(e.mess.c_str()));
      //return Undefined();
    }
    
  }
  
  
  
  Handle<Value> fits::open(const Arguments& args) {
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->check_file_is_open(args);
    return args.This();
  }

  string fits::get_file_name(const Arguments& args) {
    Handle<Value> fff=args.This()->Get(String::NewSymbol("file_name"));
    v8::String::Utf8Value fffu(fff->ToString());
    return (*fffu);
  }
  
  Handle<Value> fits::set_file(const Arguments& args) {
    HandleScope scope;
    //    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      //return scope.Close(Undefined());
    }

    Handle<Value> fff=args.This()->Get(String::NewSymbol("file_name"));
    fff=args[0]->ToString();
    
    //obj->file_name=obj->get_file_name(args);
    

    //  v8::String::Utf8Value fits_file_path(args[0]->ToString());
    //obj->file_name=*fits_file_path;
  
    return scope.Close(args.This());
  }


  v8::Handle<v8::Object> fits::get_headers_array(){
  
    //  v8::Handle<v8::Object> result = v8::Object::New();
    v8::Local<v8::Array> hdus = v8::Array::New();


    int hdupos, nkeys, nhdu, ii,ih, hdutype;

    char kname[FLEN_KEYWORD];
    char kvalue[FLEN_VALUE];
    char kcomment[FLEN_COMMENT];

  
    fits_get_num_hdus(f, &nhdu,&fstat); report_fits_error();  /* Get the current HDU position */
    fits_get_hdu_num(f, &hdupos);  /* Get the current HDU position */
    fits_get_hdrspace(f, &nkeys, NULL, &fstat); /* get # of keywords */
  
    for (ih = 1; ih <= nhdu; ih++) { /* Read and print each hdu */

      fits_movabs_hdu(f,ih,&hdutype,&fstat); report_fits_error();

      v8::Handle<v8::Object> hdu = v8::Object::New();
      v8::Local<v8::Array> fits_keys = v8::Array::New();

      hdus->Set(v8::Number::New(ih-1), hdu);
    
      for (ii = 1; ii <= nkeys; ii++) { /* Read and print each keywords */
	fits_read_keyn(f, ii, kname, kvalue, kcomment, &fstat); report_fits_error();
      
	//v8::Handle<v8::Object> key = v8::Object::New();
	v8::Local<v8::Array> key = v8::Array::New();

	key->Set(v8::Number::New(0),String::New( kname));
	key->Set(v8::Number::New(1),String::New( kvalue));
	key->Set(v8::Number::New(2),String::New( kcomment));
      
	fits_keys->Set(v8::Number::New(ii-1), key);
      }

      string type_name="bug";
      if(hdutype==IMAGE_HDU) type_name="image";
      else if(hdutype==ASCII_TBL) type_name="ascii_table";
      else if(hdutype==BINARY_TBL) type_name="binary_table";

      hdu->Set(String::New("type"),String::New(type_name.c_str()));  
      hdu->Set(String::New("keywords"),fits_keys);

    }
    
    //close_file();
    return hdus;
  }

  Handle<Value> fits::get_headers(const Arguments& args) {

    HandleScope scope;

    if (args.Length() < 1) {
      return ThrowException(Exception::Error(String::New("You need to provide a callback function(error, headers)")));
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    v8::Local<v8::Function> cb=Local<Function>::Cast(args[0]);
    //  v8::Handle<v8::Object> result = v8::Object::New();
    v8::Local<v8::Array> hdus = v8::Array::New();
    

    int hdupos, nkeys, nhdu, ii,ih, hdutype;

    char kname[FLEN_KEYWORD];
    char kvalue[FLEN_VALUE];
    char kcomment[FLEN_COMMENT];

    try{

      //      obj->open_file(obj->get_file_name(args));
      
      obj->check_file_is_open(args);

      fits_get_num_hdus(obj->f, &nhdu,&obj->fstat); obj->report_fits_error();  /* Get the current HDU position */
      fits_get_hdu_num(obj->f, &hdupos);  /* Get the current HDU position */
      fits_get_hdrspace(obj->f, &nkeys, NULL, &obj->fstat); /* get # of keywords */
      
      for (ih = 1; ih <= nhdu; ih++) { /* Read and print each hdu */
	
	fits_movabs_hdu(obj->f,ih,&hdutype,&obj->fstat); obj->report_fits_error();
	
	v8::Handle<v8::Object> hdu = v8::Object::New();
	v8::Local<v8::Array> fits_keys = v8::Array::New();
	
	hdus->Set(v8::Number::New(ih-1), hdu);
	
	for (ii = 1; ii <= nkeys; ii++) { /* Read and print each keywords */
	  fits_read_keyn(obj->f, ii, kname, kvalue, kcomment, &obj->fstat); obj->report_fits_error();
	  
	  //v8::Handle<v8::Object> key = v8::Object::New();
	  v8::Local<v8::Array> key = v8::Array::New();
	  
	  key->Set(v8::Number::New(0),String::New( kname));
	  key->Set(v8::Number::New(1),String::New( kvalue));
	  key->Set(v8::Number::New(2),String::New( kcomment));
	  
	  fits_keys->Set(v8::Number::New(ii-1), key);
	}
	
	string type_name="bug";
	if(hdutype==IMAGE_HDU) type_name="image";
	else if(hdutype==ASCII_TBL) type_name="ascii_table";
	else if(hdutype==BINARY_TBL) type_name="binary_table";

	hdu->Set(String::New("type"),String::New(type_name.c_str()));  
	hdu->Set(String::New("keywords"),fits_keys);
	
      }
      
      //      obj->close_file();

      const unsigned argc = 2;
      Handle<Value> argv[argc] = { Undefined(), hdus };
      cb->Call(Context::GetCurrent()->Global(), argc, argv );    

    }

    catch (qk::exception &e){

      const unsigned argc = 2;

      Handle<Value> argv[argc] = { String::New(e.mess.c_str()), Undefined() };
      cb->Call(Context::GetCurrent()->Global(), argc, argv );    
    }

    return scope.Close(args.This());

  }

  Handle<Value> fits::get_headers_array(const Arguments& args) {

    HandleScope scope;
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //    obj->file_name=obj->get_file_name(args);
    obj->check_file_is_open(args);

    return scope.Close(obj->get_headers_array());
  }




  v8::Handle<v8::Value> fits::set_hdu(const v8::Arguments& args){

    HandleScope scope;

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //obj->file_name=obj->get_file_name(args);

    int hduid = args[0]->ToNumber()->Value();
    MINFO << "Setting DU id to " << hduid << endl;

    //Handle<Array> cutsa = Handle<Array>::Cast(args[0]);
    try{
      //      obj->file_name=obj->get_file_name(args);
      //     obj->open_file(0);
      obj->check_file_is_open(args);
      obj->set_current_hdu(hduid);
      //cout << "setting hdu to " << hduid << "OK!!!!!"<<endl;
      return Handle<Object>(args.This());
    }

    
    catch (qk::exception& e){
      v8::ThrowException(v8::String::New(e.mess.c_str()));
      return scope.Close(Undefined());
    }

  }


  Handle<Value> fits::get_table_column(const v8::Arguments& args){
    
    v8::HandleScope scope;
    

    if (args.Length() < 2) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //    obj->file_name=obj->get_file_name(args);
    obj->check_file_is_open(args);

    return scope.Close(obj->get_table_column(args[0]->NumberValue(), args[1]->NumberValue()));
  }
  
  Handle<Object> fits::get_table_column(int hdu_id, int column_id){

    v8::Handle<v8::Object> result_object = v8::Object::New();
    
        
    column_id++;
    hdu_id++;
    
    try{

      //open_file();

      int hdutype;

      fits_movabs_hdu(f, hdu_id, NULL, &fstat); report_fits_error();    
      fits_get_hdu_type(f, &hdutype, &fstat); report_fits_error();
      

      if(hdutype == IMAGE_HDU){
	throw qk::exception("The extension is not a table");
      }
      
      long nrows; int ncols;

      fits_get_num_rows(f, &nrows, &fstat);report_fits_error(); 
      fits_get_num_cols(f, &ncols, &fstat);report_fits_error();

      char column_name[128];
      char column_id_s[32]; sprintf(column_id_s,"%d",column_id);
      int another_column_id;
      
      int col_type;
      long repeat,width;
      int anynul=0,jj;
      
      fits_get_coltype(f,column_id, &col_type,&repeat,&width, &fstat);report_fits_error(); 
      fits_get_colname(f,CASEINSEN,column_id_s,column_name, &another_column_id, &fstat);report_fits_error(); 
      
      v8::Local<v8::Array> column_data = v8::Array::New();


      result_object->Set(String::New("name"),String::New(column_name));
      
      if(col_type == TSTRING){
	result_object->Set(String::New("type"),String::New("text"));
	
	char cell_data[width+8];
	for (jj = 1; jj <= nrows && !fstat; jj++) {
	  
	  fits_read_col(f,TSTRING,column_id,jj,1,1,(char*)"NullFitsCell", cell_data, &anynul, &fstat);report_fits_error(); 
	  column_data->Set(Number::New(jj-1),String::New(cell_data));
	}
      }
      else{

	result_object->Set(String::New("type"),String::New("numerical"));
	
	double x, nulval=-9999;
	for (jj = 1; jj <= nrows && !fstat; jj++) {

	  fits_read_col(f,TDOUBLE,column_id,jj,1,1,&nulval,&x, &anynul, &fstat);report_fits_error(); 
	  column_data->Set(Number::New(jj-1),Number::New(x));
	}
      }


      result_object->Set(String::New("data"),column_data);
      close_file();
    }
      
    catch (qk::exception& e){
      v8::ThrowException(v8::String::New(e.mess.c_str()));
    }

    return result_object;

  }



  Handle<Value> fits::get_table_columns(const v8::Arguments& args){

    v8::HandleScope scope;

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->check_file_is_open(args);

    //obj->file_name=obj->get_file_name(args);
    
    return scope.Close(obj->get_table_columns(args[0]->NumberValue()));
  }


  void fits::send_status_message(v8::Local<v8::Function>& cb,const string& type, const string& message){
    const unsigned argc = 1;

    v8::Handle<v8::Object> msg = v8::Object::New();
    msg->Set(String::New(type.c_str()),String::New(message.c_str()));  

    v8::Handle<v8::Value> msgv(msg);
    Handle<Value> argv[argc] = { msgv };
    cb->Call(Context::GetCurrent()->Global(), argc, argv );    
  }
  
  
  Handle<Value> fits::read_image_hdu(const v8::Arguments& args){

    if (args.Length() < 1) {
      return ThrowException(Exception::Error(String::New("You need to provide a callback function(error, image)")));
    }
    
    v8::HandleScope scope;

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //obj->file_name=obj->get_file_name(args);

    MINFO << "Reading from FITS pointer = " << obj->f <<  " status is " << obj->fstat <<  endl;

    v8::Local<v8::Function> cb=Local<Function>::Cast(args[0]);

    try{
      obj->check_file_is_open(args);

     //obj->open_file(0);
      
      int bitpix=0;
      int eqbitpix=0;
      

      fits_get_img_type(obj->f,&bitpix,&obj->fstat); obj->report_fits_error();

      fits_get_img_equivtype(obj->f,&eqbitpix,&obj->fstat); obj->report_fits_error();

      vec<char> nulval(15);
      nulval.set_all(0);
      int anynul=0;

      int ftype=obj->get_equiv_image_type(eqbitpix);

      
      mem<long> hdd;
      obj->get_img_hdu_size(hdd);
      Handle<Value> m=obj->create_matrix_type(ftype, hdd);
      
      if(m.IsEmpty()){
	MERROR << "Empty handle !" << endl;
	return scope.Close(Undefined());
      }
      
      Handle<Object> mo=Handle<Object>::Cast(m);
      ObjectWrap* ow=  static_cast<ObjectWrap*>(mo->GetPointerFromInternalField(0));
      cnt* image_data = dynamic_cast<cnt*>(ow);
      
      long nel=image_data->nel();
      
      //      MINFO << "Creating matrix cntptr="<< image_data <<" OK  N="<< image_data->nel() << endl;
      
      fits_read_img(obj->f, ftype, 1, nel, nulval.c, image_data->data_pointer(), &anynul, &obj->fstat);
      obj->report_fits_error();
      // MINFO << "Ok image read dim =" << image_data->nel()<<endl; 

      
      const unsigned argc = 2;
      Handle<Value> argv[argc] = { Undefined(), m };
      cb->Call(Context::GetCurrent()->Global(), argc, argv );    

      return scope.Close(args.This());
    }

    catch (qk::exception &e){

      const unsigned argc = 2;

      Handle<Value> argv[argc] = { String::New(e.mess.c_str()), Undefined() };
      cb->Call(Context::GetCurrent()->Global(), argc, argv );    
    }
    
    return scope.Close(Undefined());
  }


  Handle<Value> fits::write_image_hdu(const v8::Arguments& args){

    v8::HandleScope scope;

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Handle<Object> ar=Handle<Object>::Cast(args[0]);

    jsmat<unsigned short>* image_data = ObjectWrap::Unwrap<jsmat<unsigned short> >(ar);
    
    obj->check_file_is_open(args);
    //obj->open_file(obj->get_file_name(args),1);

    obj->write_image(*image_data);

    //obj->close_file();

    return scope.Close(args.This());
  }
  
  Handle<Object> fits::get_table_columns(int hdu_id){
    
    v8::Handle<v8::Object> result_object = v8::Object::New();

    hdu_id++;
    
    try{

      //open_file();

      int hdutype;

      fits_movabs_hdu(f, hdu_id, NULL, &fstat); report_fits_error();    
      fits_get_hdu_type(f, &hdutype, &fstat); report_fits_error();
      

      if(hdutype == IMAGE_HDU){
	throw qk::exception("The extension is not a table");
      }
      
      long nrows; int ncols;

      fits_get_num_rows(f, &nrows, &fstat);report_fits_error(); 
      fits_get_num_cols(f, &ncols, &fstat);report_fits_error();

      char column_name[128];
      char column_id_s[32]; 
      int another_column_id;
      
      int col_type, column_id;
      long repeat,width;
      
      result_object->Set(String::New("nrows"),Number::New(nrows));
      v8::Local<v8::Array> columns = v8::Array::New();

      result_object->Set(String::New("columns"),columns);

      for(column_id=1; column_id<=ncols; column_id++){
	
	sprintf(column_id_s,"%d",column_id);

	v8::Handle<v8::Object> col_object = v8::Object::New();

	fits_get_coltype(f,column_id, &col_type,&repeat,&width, &fstat);report_fits_error(); 
	fits_get_colname(f,CASEINSEN,column_id_s,column_name, &another_column_id, &fstat);report_fits_error(); 
	
	col_object->Set(String::New("name"),String::New(column_name));
	col_object->Set(String::New("width"),Number::New(width));
	
	if(col_type == TSTRING){
	  col_object->Set(String::New("type"),String::New("text"));
	}
	else{
	  col_object->Set(String::New("type"),String::New("numerical"));
	}

	columns->Set(Number::New(column_id-1),col_object);
      }
      
      close_file();
    }
    
    catch (qk::exception& e){
      v8::ThrowException(v8::String::New(e.mess.c_str()));
    }

    return result_object;

  }

  void fits::Init(Handle<Object> target) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    
    s_ctf = Persistent<FunctionTemplate>::New(tpl);

    s_ctf->Inherit(colormap_interface::s_ct); 
    s_ctf->InstanceTemplate()->SetInternalFieldCount(8);
    s_ctf->SetClassName(String::NewSymbol("fits"));

    NODE_SET_PROTOTYPE_METHOD(s_ctf, "open", open);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_headers", get_headers);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "set_hdu",set_hdu);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "set_file",set_file);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_table_column",get_table_column);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_table_columns",get_table_columns);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "write_image_hdu",write_image_hdu);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "read_image_hdu",read_image_hdu);

    target->Set(String::NewSymbol("file"), s_ctf->GetFunction());
    constructor = Persistent<Function>::New(tpl->GetFunction());
  }
}



namespace sadira{
}

void init(Handle<Object> exports) {

  sadira::colormap_interface::init(exports);
  sadira::fits::Init(exports);

  //some javascriptized quarklib objects

  sadira::jsmat<int>::init(exports, "mat_int");
  sadira::jsmat<float>::init(exports, "mat_float");
  sadira::jsmat<double>::init(exports, "mat_double");
  sadira::jsmat<char>::init(exports, "mat_char");
  sadira::jsmat<unsigned int>::init(exports, "mat_uint");
  sadira::jsmat<long>::init(exports, "mat_long");
  sadira::jsmat<short>::init(exports, "mat_short");
  sadira::jsmat<unsigned char>::init(exports, "mat_uchar");
  sadira::jsmat<unsigned short>::init(exports, "mat_ushort");

  sadira::jsvec<int>::init(exports,"vec_int");
  sadira::jsvec<float>::init(exports,"vec_float");
  /* etc.... */

}

NODE_MODULE(fits, init)
