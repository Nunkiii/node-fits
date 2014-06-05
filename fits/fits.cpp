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
    fstat=0;
    f=0;
    file_name="";
    ffitsio_error = fmemopen(fitsio_error_buffer,sizeof(fitsio_error_buffer),"w");
  }
  
  fits::~fits() {
    close_file();
    fclose(ffitsio_error);
  }
  
  void fits::open_file(int _mode){
    if(f!=0) return;

    if(file_name==""){
      throw qk::exception("No file name set, use set_file command to provide one");
    }
    //    MINFO << "Opening File name [" << file_name << "]"<<endl;
    
    fstat=0;
    FILE*fff;
    
    switch (_mode){
    case 0:
      fits_open_file(&f, file_name.c_str(), READONLY, &fstat);
      break;
    case 1:
      fff=fopen(file_name.c_str(),"r"); 
      if (fff){
	fclose(fff);
	
	//Checking if file exists and user didn't provide a !, we try to delete the file
	if(file_name[0]!='!'){
	  unlink(file_name.c_str());
	  fits_create_file(&f, file_name.c_str(), &fstat);
	}else
	  fits_open_file(&f, file_name.c_str(),READWRITE, &fstat);
	
      }
      else
	fits_create_file(&f, file_name.c_str(), &fstat);
      break;
    case 2:
      fits_open_file(&f, file_name.c_str(),READWRITE, &fstat);
      break;
    default:
      break;
      
    };
    
    report_fits_error();
  }


  void fits::check_file_is_open(){
    if(!f)
      throw qk::exception("Fits file is not opened !");
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
    check_file_is_open();
    
    //if(c_hdu!=_hdu_id){
    fstat=0;
    ffmahd (f, _hdu_id+1, NULL, &fstat);  
    c_hdu=_hdu_id;
    report_fits_error();
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

    fits* obj = new fits();
    //  obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();

    string fn;

    if(!args[0]->IsUndefined()){
      v8::String::Utf8Value s(args[0]->ToString());
      fn=*s;
    }else
      fn="default.fits";

    args.This()->Set(String::NewSymbol("file_name"), String::New(fn.c_str()));
  

    obj->Wrap(args.This());

    return args.This();
  }


  string fits::get_file_name(const Arguments& args) {
    Handle<Value> fff=args.This()->Get(String::NewSymbol("file_name"));
    v8::String::Utf8Value fffu(fff->ToString());
    return (*fffu);
  }
  
  Handle<Value> fits::set_file(const Arguments& args) {
    HandleScope scope;
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      //return scope.Close(Undefined());
    }

    obj->file_name=obj->get_file_name(args);
    

    //  v8::String::Utf8Value fits_file_path(args[0]->ToString());
    //obj->file_name=*fits_file_path;
  
    return scope.Close(args.This());
  }


  v8::Handle<v8::Object> fits::get_headers(){
    
    //  v8::Handle<v8::Object> result = v8::Object::New();
    v8::Local<v8::Array> hdus = v8::Array::New();
    

    int hdupos, nkeys, nhdu, ii,ih, hdutype;

    char kname[FLEN_KEYWORD];
    char kvalue[FLEN_VALUE];
    char kcomment[FLEN_COMMENT];
  
    open_file();
  
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
  
    close_file();
    return hdus;
  }


  v8::Handle<v8::Object> fits::get_headers_array(){
  
    //  v8::Handle<v8::Object> result = v8::Object::New();
    v8::Local<v8::Array> hdus = v8::Array::New();


    int hdupos, nkeys, nhdu, ii,ih, hdutype;

    char kname[FLEN_KEYWORD];
    char kvalue[FLEN_VALUE];
    char kcomment[FLEN_COMMENT];
  
    open_file();
  
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
  
    close_file();
    return hdus;
  }

  Handle<Value> fits::get_headers(const Arguments& args) {

    HandleScope scope;


    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);

    return scope.Close(obj->get_headers());


  }

  Handle<Value> fits::get_headers_array(const Arguments& args) {

    HandleScope scope;


    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);

    return scope.Close(obj->get_headers());


  }


  Handle<Value> fits::gen_histogram(const Arguments& args) {


    //cout << "gen histo for " << obj->file_name << endl;

    HandleScope scope;

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }

    Local<Object> ar = args[0]->ToObject();
    /*
      Local<Array> props = ar->GetPropertyNames();

      for(unsigned int p=0; p<props->Length();p++){
      String::AsciiValue val(props->Get(p)->ToString());
      cout << " pr " << p << " = " << *val << endl;
      }
    */
    Local<Array> cuts_array = Local<Array>::Cast(ar->Get(String::New("cuts")));

    double cuts[2];
    cuts[0]= cuts_array->Get(0)->ToNumber()->Value();
    cuts[1]= cuts_array->Get(1)->ToNumber()->Value();

  

    //String::Utf8Value fits_file_path(args[0]->ToString());
    //Handle<Object> cuts = (*args[0])["cuts"];
    //double cut0 = args[0]["cuts"];


    //  Handle<Object> result = Object::New();
    Handle<String> histo_csv_data = obj->create_image_histogram(cuts);

    //  cout << " HISTO OK : " << *(String::AsciiValue(histo_csv_data->ToString())) << endl;

    return scope.Close(histo_csv_data);
  }


  Handle<String> fits::create_image_histogram(double* cuts){

    try{

      string result_string;
      int img_hdu_ndims=0;
      mem<long> img_hdu_dims;
      mem<long> fpix;
      float nulv=0.0f;
      // float value;
      int anynul;
      //int longest_dim=0;

      //cout << "Creating histogram ..." << endl;
    
      open_file();
      fits_get_img_dim(f, &img_hdu_ndims, &fstat);report_fits_error();

      img_hdu_dims.redim(img_hdu_ndims);
    
      if(img_hdu_ndims!=2){
	throw qk::exception("Unsupported number of dimensions in image, aborting.");
      }
    
      fpix.redim(img_hdu_ndims);
      for(int d=0;d<img_hdu_ndims;d++) fpix[d]=1;

      //cout << "ndims="<< img_hdu_ndims  << endl;
    
      fits_get_img_size(f, img_hdu_ndims, img_hdu_dims.c, &fstat);report_fits_error();

      mat<float> imgdata(img_hdu_dims[0],img_hdu_dims[1]);

      //cout << "read image : " << img_hdu_dims[0] << ", " << img_hdu_dims[1] << endl;
    
      fits_read_pix(f, TFLOAT, fpix.c, imgdata.dim ,&nulv,imgdata.c, &anynul, &fstat);report_fits_error();
    
      //cout << "OK" << endl;

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

      close_file();
    
      //cout << "OK4" << endl;
    
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


  v8::Handle<v8::Value> fits::set_hdu(const v8::Arguments& args){

    HandleScope scope;

    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    //obj->file_name=obj->get_file_name(args);

    int hduid = args[0]->ToNumber()->Value();
    cout << "setting hdu to " << hduid << endl;

    //Handle<Array> cutsa = Handle<Array>::Cast(args[0]);
    try{
      obj->file_name=obj->get_file_name(args);
      obj->open_file(0);
      obj->set_current_hdu(hduid);
      //cout << "setting hdu to " << hduid << "OK!!!!!"<<endl;
      return Handle<Object>(args.This());
    }

    
    catch (qk::exception& e){
      v8::ThrowException(v8::String::New(e.mess.c_str()));
      return scope.Close(Undefined());
    }

  }

  v8::Handle<v8::Value> fits::gen_pngtile(const v8::Arguments& args) {

    v8::HandleScope scope;
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);

    Handle<Array> parameters = Handle<Array>::Cast(args[0]);
    //  cout << " zoom = " << parameters->Get(0)->ToNumber()->Value() << endl;

    try{
      Handle<node::Buffer> bp = obj->gen_pngtile(parameters);
      return scope.Close(bp->handle_);  
    }
  
    catch (qk::exception& e){
      //    MERROR << "Failed: "<<e.mess << endl;
      v8::ThrowException(v8::String::New(e.mess.c_str()));
    }
    //  v8::Handle<v8::Object> result = v8::Object::New();
    return scope.Close(Undefined());
  }

  /*
    void free_stream_buffer(char* b, void*x){
    free(b);
    }

  */



  v8::Handle<node::Buffer> fits::gen_pngtile( Handle<Array>& parameters) {

    //  float fits_start[2];
    // float fits_size[2];
    //int png_dims[2];
    int x_tile,y_tile,zoom;

    x_tile=parameters->Get(0)->ToNumber()->Value();
    y_tile=parameters->Get(1)->ToNumber()->Value();
    zoom=parameters->Get(2)->ToNumber()->Value();

    //  cout << "Gen tile " << x_tile << ", " << y_tile << " zoom " << zoom << endl; 

    int img_hdu_ndims=0;
    mem<long> img_hdu_dims;
    mem<long> fpix;
    float nulv=0.0f;
    float value;
    int anynul;
    int longest_dim=0;

    dcube<unsigned char> png_data(256,256,4);
    png_data.set_all(0);

    float scale_factor;
    
    open_file();
  
    //  cout << "OK f="<< f << endl;
    fits_get_img_dim(f, &img_hdu_ndims, &fstat); report_fits_error();

    if(img_hdu_ndims!=2){
      throw qk::exception("Unsupported number of dimensions in image, aborting.");
    }

  
    //cout << "OK" << endl;
  
    img_hdu_dims.redim(img_hdu_ndims);
    fpix.redim(img_hdu_ndims);
  
    fits_get_img_size(f, img_hdu_ndims, img_hdu_dims.c, &fstat);report_fits_error();
  
    //cout << "OK" << endl;

    for(int d=0;d<img_hdu_ndims;d++) fpix[d]=1;
  
    if(img_hdu_dims[1]>img_hdu_dims[0]) longest_dim=1;

    //cout << "OK" << endl;  
    //longest_dim=0;
  
    scale_factor=img_hdu_dims[longest_dim]*1.0/png_data.dims[longest_dim];
    for(int z=0;z<zoom;z++) scale_factor/=2.0;
  
    //  cout << "Fits data :  " << img_hdu_dims.dim << " dimensions : ";
    // for(int d=0;d<img_hdu_dims.dim;d++) cout << img_hdu_dims[d] << ", ";
    // cout << endl;

  
    vec<float> tmpcol(4);
  
    for(int y=0;y<png_data.dims[1];y++)
      for(int x=0;x<png_data.dims[0];x++){
      
	fpix[0]=(x_tile*png_data.dims[0]+x)*scale_factor+1;
	fpix[1]=(y_tile*png_data.dims[1]+y)*scale_factor+1;
      
	if(fpix[0]<0 || fpix[0]>img_hdu_dims[0]
	   || fpix[1]<0 || fpix[1]>img_hdu_dims[1]
	   ) value=cuts[1];
      
	else{
	  fits_read_pix(f, TFLOAT, fpix.c, 1,&nulv,&value, &anynul, &fstat);
	  report_fits_error();
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
  
    close_file();    
  
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


  Handle<Value> fits::get_table_column(const v8::Arguments& args){
    
    v8::HandleScope scope;
    

    if (args.Length() < 2) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
    }
    
    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);
    
    return scope.Close(obj->get_table_column(args[0]->NumberValue(), args[1]->NumberValue()));
  }
  
  Handle<Object> fits::get_table_column(int hdu_id, int column_id){

    v8::Handle<v8::Object> result_object = v8::Object::New();
    
        
    column_id++;
    hdu_id++;
    
    try{

      open_file();

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
    obj->file_name=obj->get_file_name(args);
    
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
    
    cout << "Hello"<<endl;
    v8::HandleScope scope;

    fits* obj = ObjectWrap::Unwrap<fits>(args.This());
    obj->file_name=obj->get_file_name(args);

    v8::Local<v8::Function> cb=Local<Function>::Cast(args[0]);

    try{
      obj->open_file(0);
      
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
      

      MINFO << "Creating matrix " << endl;
      
      Handle<Value> m=obj->create_matrix_type(ftype, hdd);
    
      
      if(m.IsEmpty()){
	MERROR << "Empty handle !" << endl;
	return scope.Close(Undefined());
      }
      
      Handle<Object> mo=Handle<Object>::Cast(m);
      ObjectWrap* ow=  static_cast<ObjectWrap*>(mo->GetPointerFromInternalField(0));
      cnt* image_data = dynamic_cast<cnt*>(ow);
      
      long nel=image_data->nel();
      
      MINFO << "Creating matrix cntptr="<< image_data <<" OK  N="<< image_data->nel() << endl;
      
      fits_read_img(obj->f, ftype, 1, nel, nulval.c, image_data->data_pointer(), &anynul, &obj->fstat);
      obj->report_fits_error();
      MINFO << "Ok image read dim =" << image_data->nel()<<endl; 

      
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
    

    obj->file_name=obj->get_file_name(args);

    cout << "Yeah we have the image data W= " << image_data->dims[0] << " opening file name " << obj->file_name << endl;
    
    obj->open_file(1);

    obj->write_image(*image_data);

    obj->close_file();

    return scope.Close(Undefined());
  }
  
  Handle<Object> fits::get_table_columns(int hdu_id){
    
    v8::Handle<v8::Object> result_object = v8::Object::New();

    hdu_id++;
    
    try{

      open_file();

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
    s_ctf->InstanceTemplate()->SetInternalFieldCount(1);
    s_ctf->SetClassName(String::NewSymbol("fits"));

    NODE_SET_PROTOTYPE_METHOD(s_ctf, "gen_histogram", gen_histogram);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_headers", get_headers);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "set_hdu",set_hdu);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "set_file",set_file);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "gen_pngtile",gen_pngtile);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_table_column",get_table_column);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "get_table_columns",get_table_columns);

    NODE_SET_PROTOTYPE_METHOD(s_ctf, "write_image_hdu",write_image_hdu);
    NODE_SET_PROTOTYPE_METHOD(s_ctf, "read_image_hdu",read_image_hdu);

    target->Set(String::NewSymbol("file"), s_ctf->GetFunction());

  }
}

namespace sadira{
  Persistent<FunctionTemplate> fits::s_ctf;
  template <class T> Persistent<FunctionTemplate> jsvec<T>::s_ctm;
  template <class T> Persistent<FunctionTemplate> jsmat<T>::s_ctm;
  template <class T> Persistent<Function> jsmat<T>::constructor;
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
