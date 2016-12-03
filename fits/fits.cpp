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

  Persistent<Function> fits::constructor;
  
  void free_stream_buffer(char* b, void*x){
    free(b);
  }


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
    //MINFO << "destructor called !" << endl;
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
    MINFO << "Opening File name [" << file_name << "] mode " << _mode <<endl;
    
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

    //MINFO << "OK FITS opened pointer = " << f << endl;
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
    //MINFO << "Switching to hdu " << _hdu_id << endl;
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
    //    //MINFO << "Getting hdu dimensions..."<<endl;
    int img_hdu_ndims=0;
    fits_get_img_dim(f, &img_hdu_ndims, &fstat);
    report_fits_error();
    
    if(fstat) return;

    hdims.redim(img_hdu_ndims);

    fits_get_img_size(f, img_hdu_ndims, hdims.c, &fstat);
    report_fits_error();

    //MINFO << "Full image HDU size " << hdims[0] << ", " << hdims[1] <<endl;
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
  


  void fits::New(const FunctionCallbackInfo<Value>& args) {

    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall()) {
      
      //MINFO << "Createing new FITS object !" << endl;

      fits* obj = new fits();
      obj->fstat=0;
      obj->f=0;
      obj->c_hdu=0;
      obj->ffitsio_error = fmemopen(obj->fitsio_error_buffer,sizeof(obj->fitsio_error_buffer),"w");
      
      if(!args[0]->IsUndefined()){
	//String::Utf8Value s(args[0]->ToString());
	//args.This()->Set(String::NewFromUtf8(isolate,"file_name"), String::New(isolate, *s));
	args.This()->Set(String::NewFromUtf8(isolate,"file_name"), args[0]);
	open(args);
      }//else args.This()->Set(String::NewFromUtf8(isolate,"file_name"), String::New(""));

      
      
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());

    }else{
      const int argc = 1;
      Local<Value> argv[argc] = { args[0] };
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      args.GetReturnValue().Set(cons->NewInstance(argc, argv));

    }
  }


  void fits::check_file_is_open(const FunctionCallbackInfo<Value>& args, int mode) {
    Isolate* isolate = args.GetIsolate();
    fits* obj = ObjectWrap::Unwrap<fits>(args.Holder());

    if(f) return;
    
    //MINFO << "Opening ..."  << endl;
    Local<Value> fff=args.This()->Get(String::NewFromUtf8(isolate,"file_name"));
    
    if(fff->IsUndefined()){
      //MINFO << "UNDEFINED"  << endl;
      throw qk::exception("File name not set !");
      //isolate->ThrowException(String::NewFromUtf8(isolate, "File name not set !"));
      //MINFO << "SENT EXC"  << endl;
      return;
    }
    
    String::Utf8Value fn(fff->ToString());
    
    //string fn=obj->get_file_name(args);
    
    //MINFO << "Opening " << *fn << endl;
    //    try{
    obj->open_file(*fn, mode);
    // }
    // catch (qk::exception& e){
    //   isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
    //   return;
    // }
    
  }
  
  
  
  void fits::open(const FunctionCallbackInfo<Value>& args) {
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->check_file_is_open(args);
    args.GetReturnValue().Set(args.This());
  }

  string fits::get_file_name(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Value> fff=args.This()->Get(String::NewFromUtf8(isolate,"file_name"));
    String::Utf8Value fffu(fff->ToString());
    return (*fffu);
  }
  
  void fits::set_file(const FunctionCallbackInfo<Value>& args) {
    
    //    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      //return scope.Close(Undefined());
    }

    Local<Value> fff=args.This()->Get(String::NewFromUtf8(isolate,"file_name"));
    fff=args[0]->ToString();
    
    //obj->file_name=obj->get_file_name(args);
    

    //  String::Utf8Value fits_file_path(args[0]->ToString());
    //obj->file_name=*fits_file_path;
    
    args.GetReturnValue().Set(args.This());
  }


  Local<Object> fits::get_headers_array(Isolate* isolate){
  
    //  Local<Object> result = Object::New(isolate);
    Local<Array> hdus = Array::New(isolate);
    int hdupos, nkeys, nhdu, ii,ih, hdutype;

    char kname[FLEN_KEYWORD];
    char kvalue[FLEN_VALUE];
    char kcomment[FLEN_COMMENT];

    fits_get_num_hdus(f, &nhdu,&fstat); report_fits_error();  /* Get the current HDU position */
    fits_get_hdu_num(f, &hdupos);  /* Get the current HDU position */
    fits_get_hdrspace(f, &nkeys, NULL, &fstat); /* get # of keywords */
  
    for (ih = 1; ih <= nhdu; ih++) { /* Read and print each hdu */

      fits_movabs_hdu(f,ih,&hdutype,&fstat); report_fits_error();

      Local<Object> hdu = Object::New(isolate);
      Local<Array> fits_keys = Array::New(isolate);

      hdus->Set(Number::New(isolate, ih-1), hdu);
    
      for (ii = 1; ii <= nkeys; ii++) { /* Read and print each keywords */
	fits_read_keyn(f, ii, kname, kvalue, kcomment, &fstat); report_fits_error();
      
	//Local<Object> key = Object::New(isolate);
	Local<Array> key = Array::New(isolate);

	key->Set(Number::New(isolate, 0),String::NewFromUtf8(isolate,  kname));
	key->Set(Number::New(isolate, 1),String::NewFromUtf8(isolate,  kvalue));
	key->Set(Number::New(isolate, 2),String::NewFromUtf8(isolate,  kcomment));
      
	fits_keys->Set(Number::New(isolate, ii-1), key);
      }

      string type_name="bug";
      if(hdutype==IMAGE_HDU) type_name="image";
      else if(hdutype==ASCII_TBL) type_name="ascii_table";
      else if(hdutype==BINARY_TBL) type_name="binary_table";

      hdu->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, type_name.c_str()));  
      hdu->Set(String::NewFromUtf8(isolate, "keywords"),fits_keys);

    }
    
    //close_file();
    return hdus;
  }

  void fits::get_headers(const FunctionCallbackInfo<Value>& args) {
    
    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 1) {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "You need to provide a callback function(error, headers)")));
      return;
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Local<Function> cb=Local<Function>::Cast(args[0]);
    //  Local<Object> result = Object::New(isolate, );
    Local<Array> hdus = Array::New(isolate);
    

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
	
	Local<Object> hdu = Object::New(isolate);
	Local<Object> fits_keys = Object::New(isolate);
	
	hdus->Set(Number::New(isolate, ih-1), hdu);
	
	for (ii = 1; ii <= nkeys; ii++) { /* Read and print each keywords */
	  fits_read_keyn(obj->f, ii, kname, kvalue, kcomment, &obj->fstat); obj->report_fits_error();
	  
	  Local<Object> key_data = Object::New(isolate);

	  double num_value; 
	  if(sscanf(kvalue,"%lf",&num_value)==1)
	    key_data->Set(String::NewFromUtf8(isolate, "value"),Number::New(isolate,  num_value));
	  else
	    key_data->Set(String::NewFromUtf8(isolate, "value"),String::NewFromUtf8(isolate,  kvalue));

	  key_data->Set(String::NewFromUtf8(isolate, "comment"),String::NewFromUtf8(isolate,  kcomment));
	  
	  fits_keys->Set(String::NewFromUtf8(isolate, kname), key_data);
	  
	  //fits_keys->Set(Number::New(ii-1), key);
	}
	
	string type_name="bug";
	if(hdutype==IMAGE_HDU) type_name="image";
	else if(hdutype==ASCII_TBL) type_name="ascii_table";
	else if(hdutype==BINARY_TBL) type_name="binary_table";

	hdu->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, type_name.c_str()));  
	hdu->Set(String::NewFromUtf8(isolate, "keywords"),fits_keys);
	
      }
      
      //      obj->close_file();

      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), hdus };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    

    }

    catch (qk::exception &e){

      const unsigned argc = 2;

      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    args.GetReturnValue().Set(args.This());

  }

  void fits::set_header_key(const FunctionCallbackInfo<Value>& args) {

    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 2) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "node-fits : Wrong number of arguments : need  a key info hash and callback function !")));
      args.GetReturnValue().Set(Undefined(isolate));
      return;
    }

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Local<Object> opts=Local<Object>::Cast(args[0]);
    Local<Function> cb=Local<Function>::Cast(args[1]);
    
    //if (callback->IsUndefined()) {

    Local<Value> key_name = opts->Get(String::NewFromUtf8(isolate, "key"));
    Local<Value> key_value = opts->Get(String::NewFromUtf8(isolate, "value"));
    Local<Value> key_comment = opts->Get(String::NewFromUtf8(isolate, "comment"));

    try{
      
      if(key_name->IsUndefined()) 
      	throw qk::exception("You need to provide a 'key' string in first argument object !");
      if(key_value->IsUndefined()) 
      	throw qk::exception("You need to provide a 'value' string in first argument object !");
      
      v8::String::Utf8Value key(key_name->ToString());
      v8::String::Utf8Value value(key_value->ToString());
      
      std::string s_key = std::string(*key);
      std::string s_value = std::string(*value);
      std::string s_comment = "";
	
      if(!key_comment->IsUndefined()){
	v8::String::Utf8Value comment(key_comment->ToString());
	s_comment = std::string(*comment);
      }

      obj->check_file_is_open(args, 2);
      obj->set_current_hdu(0);
      obj->write_key_str(s_key, s_value, s_comment);
      obj->close_file();
      
      
      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    catch (qk::exception &e){

      const unsigned argc = 2;
      
      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }
    

    args.GetReturnValue().Set(args.This());
  }
  
  void fits::get_headers_array(const FunctionCallbackInfo<Value>& args) {

    Isolate* isolate = args.GetIsolate();
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //    obj->file_name=obj->get_file_name(args);
    obj->check_file_is_open(args);
    
    args.GetReturnValue().Set(obj->get_headers_array(isolate));
  }




  void fits::set_hdu(const FunctionCallbackInfo<Value>& args){

    
    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      args.GetReturnValue().Set(Undefined(isolate));
    }

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //obj->file_name=obj->get_file_name(args);

    int hduid = args[0]->ToNumber()->Value();
    //MINFO << "Setting DU id to " << hduid << endl;

    //Local<Array> cutsa = Local<Array>::Cast(args[0]);
    try{
      //      obj->file_name=obj->get_file_name(args);
      //     obj->open_file(0);
      obj->check_file_is_open(args);
      obj->set_current_hdu(hduid);
      //cout << "setting hdu to " << hduid << "OK!!!!!"<<endl;
      args.GetReturnValue().Set(args.This());
    }
    
    catch (qk::exception& e){
      isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
      args.GetReturnValue().Set(Undefined(isolate));
    }

  }


  void fits::get_table_column(const FunctionCallbackInfo<Value>& args){
    
    //cout << "Get table column " <<  endl;

    
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      args.GetReturnValue().Set(Undefined(isolate));
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //    obj->file_name=obj->get_file_name(args);


    Local<Function> cb=Local<Function>::Cast(args[1]);

    try{
      obj->check_file_is_open(args);

      Local<Object> column=obj->get_table_column(isolate, args[0]->NumberValue());
      
      //args.GetReturnValue().Set(obj->get_table_columns(args[0]->NumberValue()));
      
      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), column };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    catch (qk::exception &e){

      const unsigned argc = 2;
      
      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    args.GetReturnValue().Set(args.This());
  }


  void fits::get_table_data(const FunctionCallbackInfo<Value>& args){
      
    Isolate* isolate = args.GetIsolate();

    HandleScope handle_scope(isolate);
    cout << "Handle scope BEGIN N=" << handle_scope.NumberOfHandles(isolate)<<endl;
    
    if (args.Length() < 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      args.GetReturnValue().Set(Undefined(isolate));
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Local<Function> cb=Local<Function>::Cast(args[0]);
    Local<Object> opts=Local<Object>::Cast(args[1]);

    Local<Object> opts_row_start=Local<Object>::Cast(opts->Get( String::NewFromUtf8(isolate, "row_start")));
    Local<Object> opts_nrows=Local<Object>::Cast(opts->Get( String::NewFromUtf8(isolate, "nrows")));

    int row_start=0;
    if ( ! opts_row_start->IsUndefined() ) row_start= (int) opts_row_start->ToNumber()->Value();

    int nrows=-1;
    if ( ! opts_row_start->IsUndefined() )  nrows= (int) opts_nrows->ToNumber()->Value();

    cout << "Row start= " << row_start << " nr=" << nrows<< endl;
    
    try{
      obj->check_file_is_open(args);

      //obj->file_name=obj->get_file_name(args);
      Local<Array> columns=obj->get_table_data(isolate, row_start, nrows);
    
      //args.GetReturnValue().Set(obj->get_table_columns(args[0]->NumberValue()));
      cout << "Got data aray L= " << columns->Length();
      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), columns };

      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );

      cout << "Handle scope N=" << handle_scope.NumberOfHandles(isolate)<<endl;
      //handle_scope->Close();
      //columns->Delete(isolate, columns);
    }

    catch (qk::exception &e){

      const unsigned argc = 2;
      
      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    args.GetReturnValue().Set(args.This());
    
  }


  Local<Array> fits::get_table_data(Isolate* isolate, int row_start, int nrows_max){


    Local<Array> result_object = Array::New(isolate);

    
    
    try{
      
      int hdutype;

      fits_get_hdu_type(f, &hdutype, &fstat); report_fits_error();
      

      if(hdutype == IMAGE_HDU){
	throw qk::exception("The extension is not a table");
      }
      
      long nrows; int ncols;

      fits_get_num_rows(f, &nrows, &fstat);report_fits_error(); 
      fits_get_num_cols(f, &ncols, &fstat);report_fits_error();
      
      char column_name[ncols][128];
      char column_id_s[32];
      int another_column_id;
      
      int col_type[ncols];
      long repeat,width=0;
      int anynul=0,jj;

      for(int k=1;k<ncols+1;k++){
	sprintf(column_id_s,"%d",k);
	long w;
	fits_get_coltype(f,k, &col_type[k-1],&repeat,&w, &fstat);report_fits_error();
	if(w>width) width=w;
	fits_get_colname(f,CASEINSEN,column_id_s,column_name[k-1], &another_column_id, &fstat);report_fits_error(); 
	//cout << "COL " << k << " : w = " << w << endl; 
      }
      
      

      char* cell_data = new char[width+8];
      char null_data[width+8];
      for(int k=0;k<width+8;k++) null_data[k]=0;
      double x, nulval=-9999;
      
      //result_object->Set(String::NewFromUtf8("name"),String::NewFromUtf8(column_name));
      //result_object->Set(String::NewFromUtf8("type"),String::NewFromUtf8("text"));

      if(nrows_max==-1) nrows_max=nrows;
      
      for (jj = row_start+1; jj <= row_start+nrows_max && !fstat; jj++) {
	
	Local<Array> row_data = Array::New(isolate);
	result_object->Set(Number::New(isolate, jj-1),row_data);
	
	//cout << "w= "<< width<<"Read row " << jj << endl;
	//for(int k=0;k<width+8;k++) cell_data[k]=0;
	//cout << "Read row " << jj << " cell data " << cell_data <<  endl;
	for(int column_id=0;column_id<ncols;column_id++){
	  if(col_type[column_id] == TSTRING){
	    fits_read_col(f,TSTRING,column_id+1,jj,1,1,&null_data, &cell_data, &anynul, &fstat);
	    report_fits_error(); 
	    //cout << " Read string ["<< cell_data <<"]" << endl; 
	    row_data->Set(Number::New(isolate, column_id),String::NewFromUtf8(isolate, cell_data));
	  }else{
	    fits_read_col(f,TDOUBLE,column_id+1,jj,1,1,&nulval,&x, &anynul, &fstat);report_fits_error(); 
	    row_data->Set(Number::New(isolate, column_id),Number::New(isolate, x));
	  }
	  
	}
      }

      delete[] cell_data;
    }
    
    catch (qk::exception& e){
      isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
    }

    return result_object;

  }


  
  Local<Object> fits::get_table_column(Isolate* isolate, int column_id){

    Local<Object> result_object = Object::New(isolate);
    
    column_id++;
    //    hdu_id++;
    
    try{

      //open_file();

      int hdutype;

      //      fits_movabs_hdu(f, hdu_id, NULL, &fstat); report_fits_error();    
      fits_get_hdu_type(f, &hdutype, &fstat); report_fits_error();
      

      if(hdutype == IMAGE_HDU){
	throw qk::exception("The extension is not a table");
      }
      
      long int nrows; int ncols;

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
      
      Local<Array> column_data = Array::New(isolate);


      result_object->Set(String::NewFromUtf8(isolate, "name"),String::NewFromUtf8(isolate, column_name));
      
      if(col_type == TSTRING){
	result_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "text"));
	
	char* cell_data = new char[width+8];
	char null_data[width+8];
	for(int k=0;k<width+8;k++) null_data[k]=0;
	
	for (jj = 1; jj <= nrows && !fstat; jj++) {
	  //cout << "w= "<< width<<"Read row " << jj << endl;
	  //for(int k=0;k<width+8;k++) cell_data[k]=0;
	  //cout << "Read row " << jj << " cell data " << cell_data <<  endl;

	  fits_read_col(f,TSTRING,column_id,jj,1,1,&null_data, &cell_data, &anynul, &fstat);
	  report_fits_error(); 
	  //cout << " Read string ["<< cell_data <<"]" << endl; 
	  column_data->Set(Number::New(isolate, jj-1),String::NewFromUtf8(isolate, cell_data));
	}
	delete[] cell_data;
      }
      else{


	//Local<Float32Array> lfa = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), nrows), 0, nrows);
	

	result_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "numerical"));
	//cout << "Alloc memory ...  " << nrows << endl;

	qk::mem<double> x(nrows);
	double nulval=-9999;	

	//cout << "Reading fits ...  " << x.dim << endl;
	fits_read_col(f,TDOUBLE,column_id,1,1,nrows,&nulval,x.c, &anynul, &fstat);

	report_fits_error();


	
	//cout << "Copy into JS array...  " << x.dim << endl;
	
	for (jj = 0; jj < nrows; jj++) {
	  //fits_read_col(f,TDOUBLE,column_id,jj,1,1,&nulval,&x, &anynul, &fstat);report_fits_error(); 
	  column_data->Set(Number::New(isolate, jj),Number::New(isolate, x[jj]));
	}
	//cout << "Copy JS array done!  " << x.dim << endl;
      }
      
      result_object->Set(String::NewFromUtf8(isolate, "data"),column_data);
      close_file();
    }
    
    catch (qk::exception& e){
      isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
    }

    return result_object;

  }



  void fits::get_table_columns(const FunctionCallbackInfo<Value>& args){

    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      args.GetReturnValue().Set(Undefined(isolate));
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Local<Value> options(args[0]);
    Local<Value> callback(args[1]);

    if (callback->IsUndefined()) {
      if (options->IsFunction()) {
	callback = options;
	options = Object::New(isolate);
      }
    }

    Local<Function> cb;
    Local<Object> opts;
    cb=Local<Function>::Cast(callback);
    opts=Local<Object>::Cast(options);

    Local<Value> extract_type = opts->Get(String::NewFromUtf8(isolate, "type"));
    int etype=0;
    
    if(!extract_type->IsUndefined()){
      Local<String> in_etype = Local<String>::Cast(extract_type);
      if(strcmp(*String::Utf8Value(in_etype),"hash")==0){
	//cout << "Got hash option!"<< endl;
	etype=1;
      }
    }
      
    
    try{
      obj->check_file_is_open(args);

      //obj->file_name=obj->get_file_name(args);

      Local<Object> columns= etype ? obj->get_table_columns_hash(isolate) : obj->get_table_columns(isolate);
    
      //args.GetReturnValue().Set(obj->get_table_columns(args[0]->NumberValue()));

      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), columns };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    catch (qk::exception &e){

      const unsigned argc = 2;
      
      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }

    args.GetReturnValue().Set(args.This());
    
  }


  void fits::send_status_message(Isolate* isolate, Local<Function>& cb,const string& type, const string& message){
    const unsigned argc = 1;

    Local<Object> msg = Object::New(isolate);
    msg->Set(String::NewFromUtf8(isolate, type.c_str()),String::NewFromUtf8(isolate, message.c_str()));  

    Local<Value> msgv(msg);
    Local<Value> argv[argc] = { msgv };
    cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
  }
  
  
  void fits::read_image_hdu(const FunctionCallbackInfo<Value>& args){
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1) {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "You need to provide a callback function(error, image)")));
      return;
    }
    
    

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //obj->file_name=obj->get_file_name(args);

    //MINFO << "Reading from FITS pointer = " << obj->f <<  " status is " << obj->fstat <<  endl;

    Local<Function> cb=Local<Function>::Cast(args[0]);

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
      
      
      Local<Value> m=obj->create_matrix_type(isolate, ftype, hdd);
      
      if(m.IsEmpty()){
	MERROR << "Empty handle !" << endl;
	args.GetReturnValue().Set(Undefined(isolate));
      }
      
      Local<Object> mo=Local<Object>::Cast(m);
      ObjectWrap* ow=  static_cast<ObjectWrap*>(mo->GetAlignedPointerFromInternalField(0));
      cnt* image_data = dynamic_cast<cnt*>(ow);

      //cnt* image_data = ObjectWrap::Unwrap<cnt>(mo);
      
      long nel=image_data->nel();
      
      //      MINFO << "Creating matrix cntptr="<< image_data <<" OK  N="<< image_data->nel() << endl;
      
      fits_read_img(obj->f, ftype, 1, nel, nulval.c, image_data->data_pointer(), &anynul, &obj->fstat);
      obj->report_fits_error();
      // MINFO << "Ok image read dim =" << image_data->nel()<<endl; 

      
      const unsigned argc = 2;
      Local<Value> argv[argc] = { Undefined(isolate), m };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    

      args.GetReturnValue().Set(args.This());
    }

    catch (qk::exception &e){

      const unsigned argc = 2;

      Local<Value> argv[argc] = { String::NewFromUtf8(isolate, e.mess.c_str()), Undefined(isolate) };
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv );    
    }
    
    args.GetReturnValue().Set(Undefined(isolate));
  }


  void fits::write_image_hdu(const FunctionCallbackInfo<Value>& args){
    
    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      args.GetReturnValue().Set(Undefined(isolate));
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    Local<Object> ar=Local<Object>::Cast(args[0]);

    jsmat<unsigned short>* image_data = ObjectWrap::Unwrap<jsmat<unsigned short> >(ar);
    
    obj->check_file_is_open(args,1);
    //obj->open_file(obj->get_file_name(args),1);

    obj->write_image(*image_data);
    obj->close_file();

    

    args.GetReturnValue().Set(args.This());
  }
  
  Local<Object> fits::get_table_columns(Isolate* isolate){
    
    Local<Object> result_object = Object::New(isolate);

    // hdu_id++;
    
    try{

      //open_file();

      int hdutype;

      //fits_movabs_hdu(f, hdu_id, NULL, &fstat); report_fits_error();    

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
      
      result_object->Set(String::NewFromUtf8(isolate, "nrows"),Number::New(isolate, nrows));
      Local<Array> columns = Array::New(isolate);
      
      result_object->Set(String::NewFromUtf8(isolate, "columns"),columns);
      
      for(column_id=1; column_id<=ncols; column_id++){
	
	sprintf(column_id_s,"%d",column_id);
	
	Local<Object> col_object = Object::New(isolate);
	
	fits_get_coltype(f,column_id, &col_type,&repeat,&width, &fstat);report_fits_error(); 
	fits_get_colname(f,CASEINSEN,column_id_s,column_name, &another_column_id, &fstat);report_fits_error(); 
	
	col_object->Set(String::NewFromUtf8(isolate, "name"),String::NewFromUtf8(isolate, column_name));
	col_object->Set(String::NewFromUtf8(isolate, "width"),Number::New(isolate, width));
	col_object->Set(String::NewFromUtf8(isolate, "coltype"),Number::New(isolate, col_type));
	
	if(col_type == TSTRING){
	  col_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "text"));
	}
	else{
	  col_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "numerical"));
	}

	columns->Set(Number::New(isolate, column_id-1),col_object);
      }
      
      close_file();
    }
    
    catch (qk::exception& e){
      isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
    }

    return result_object;

  }

  Local<Object> fits::get_table_columns_hash(Isolate* isolate){
    
    Local<Object> result_object = Object::New(isolate);

    // hdu_id++;
    
    try{

      //open_file();

      int hdutype;

      //fits_movabs_hdu(f, hdu_id, NULL, &fstat); report_fits_error();    

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
      
      result_object->Set(String::NewFromUtf8(isolate, "nrows"),Number::New(isolate, nrows));
      Local<Object> columns = Object::New(isolate);
      
      result_object->Set(String::NewFromUtf8(isolate, "columns"),columns);
      
      for(column_id=1; column_id<=ncols; column_id++){
	
	sprintf(column_id_s,"%d",column_id);
	
	Local<Object> col_object = Object::New(isolate);
	
	fits_get_coltype(f,column_id, &col_type,&repeat,&width, &fstat);report_fits_error(); 
	fits_get_colname(f,CASEINSEN,column_id_s,column_name, &another_column_id, &fstat);report_fits_error(); 

	col_object->Set(String::NewFromUtf8(isolate, "id"),Number::New(isolate, column_id-1));
	col_object->Set(String::NewFromUtf8(isolate, "name"),String::NewFromUtf8(isolate, column_name));
	col_object->Set(String::NewFromUtf8(isolate, "width"),Number::New(isolate, width));
	
	if(col_type == TSTRING){
	  col_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "text"));
	}
	else{
	  col_object->Set(String::NewFromUtf8(isolate, "type"),String::NewFromUtf8(isolate, "numerical"));
	}

	columns->Set(String::NewFromUtf8(isolate, column_name),col_object);
      }
      
      close_file();
    }
    
    catch (qk::exception& e){
      isolate->ThrowException(String::NewFromUtf8(isolate, e.mess.c_str()));
    }

    return result_object;

  }


  
  void fits::Init(Local<Object> target) {

    Isolate* isolate=target->GetIsolate();
      
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    
    
    tpl->InstanceTemplate()->SetInternalFieldCount(9);
    tpl->SetClassName(String::NewFromUtf8(isolate,"fits"));

    NODE_SET_PROTOTYPE_METHOD(tpl, "open", open);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_headers", get_headers);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_headers_array", get_headers_array);
    NODE_SET_PROTOTYPE_METHOD(tpl, "set_hdu",set_hdu);
    NODE_SET_PROTOTYPE_METHOD(tpl, "set_file",set_file);
    NODE_SET_PROTOTYPE_METHOD(tpl, "set_header_key",set_header_key);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_table_column",get_table_column);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_table_columns",get_table_columns);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_table_data",get_table_data);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write_image_hdu",write_image_hdu);
    NODE_SET_PROTOTYPE_METHOD(tpl, "read_image_hdu",read_image_hdu);

          
    target->Set(String::NewFromUtf8(isolate,"file"), tpl->GetFunction());
    constructor.Reset(isolate, tpl->GetFunction());
  }

}


void init(Local<Object> exports) {

  //  sadira::colormap_interface::init(exports);


  sadira::fits::Init(exports);

  //  return;

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
  sadira::jsvec<float>::init(exports,"vec_double");


}

NODE_MODULE(fits, init)
