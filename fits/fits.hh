/*
- 2003 - 2014 - The Qk/Sadira project - P.G. Sprimont @ INAF, Bologna, IT. 
*/

#ifndef __FITS_HH__
#define __FITS_HH__

#include <node.h>
#include <node_buffer.h>
#include <string>
#include <fitsio.h>

//#include <qk/colormap.hh>
#include <math/jsmat.hh>
#include <math/jsvec.hh>

namespace sadira{
  
  using namespace std;
  using namespace v8;
  using namespace qk;
  
  class fits : public node::ObjectWrap {

  public:

    static void Init(Handle<Object> exports);
    static Persistent<FunctionTemplate> s_ctf;
    
  private:
    
    static v8::Persistent<v8::Function> constructor;
    
    fits();
    ~fits();
    
    void open_file(const char* file_name, int mode=0);
    void close_file();
    Handle<Object> get_headers_array();

    void write_key_str(const string& _keyword, const string& _key_val, const string& _comment);
    void read_key_str(const string& _key_name,string& _key_val);
    
    int get_num_hdu();
    void set_current_hdu(int _hdu_id);
    int get_current_hdu();
    int get_hdu_type();
    void get_img_hdu_size(mem<long>&hdims);

    void send_status_message(v8::Local<v8::Function>& cb,const string& type, const string& message);

    void check_file_is_open(const Arguments& args);

    //    Handle<node::Buffer> gen_pngtile(Handle<Array>& parameters);
    
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> open(const Arguments& args);

    static Handle<Value> set_file(const Arguments& args);
    static Handle<Value> get_headers(const Arguments& args);
    static Handle<Value> get_headers_array(const Arguments& args);
    //    static Handle<Value> gen_pngtile(const Arguments& args);


    static Handle<Value> write_image_hdu(const v8::Arguments& args);
    static Handle<Value> read_image_hdu(const v8::Arguments& args);
    
    static Handle<Value> set_hdu(const Arguments& args);

    Handle<Object> get_table_column(int column_id);
    static Handle<Value> get_table_column(const Arguments& args);


    static Handle<Value> get_table_columns(const Arguments& args);
    Handle<Object> get_table_columns();
    Handle<Object> get_table_columns_hash();

    static Handle<Value> get_table_data(const Arguments& args);
    Handle<Array> get_table_data();
    
    void report_fits_error();

    //    Handle<String> create_image_histogram(double* cuts);
    //    static Handle<Value> gen_histogram(const Arguments& args);

    double counter_;

    string get_file_name(const Arguments& args);
    
    template <typename T>

    void write_key(T& _numobj, const string& _comment=""){
      int ft=get_fits_type<T>();
      ffpky(f, ft,  _numobj.get_name().c_str(), &_numobj, _comment.c_str(), &fstat);
      report_fits_error();
    }
    
    
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

      MINFO << "writing image types="<< fits_image_type << ", "<< fits_type << endl;
      
      fits_create_img(f, fits_image_type, 2, ndims, &fstat);
      report_fits_error();

      fits_write_subset(f, fits_type, fpix, ndims,_m.c, &fstat);
      report_fits_error();
      
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

    
    Handle<Value> create_matrix_type(int _ftype, mem<long>&hdd){
            
      switch (_ftype){
      case TBYTE: return jsmat<unsigned char>::Instantiate(hdd[0],hdd[1]);
      case TSBYTE: return jsmat<char>::Instantiate(hdd[0],hdd[1]);
      case TSHORT: return jsmat<short int>::Instantiate(hdd[0],hdd[1]);
      case TUSHORT: return jsmat<unsigned short int>::Instantiate(hdd[0],hdd[1]);
      case TLONG: return jsmat<int>::Instantiate(hdd[0],hdd[1]);
      case TULONG: return jsmat<unsigned int>::Instantiate(hdd[0],hdd[1]);
      case TLONGLONG: return jsmat<long int>::Instantiate(hdd[0],hdd[1]);
      case TDOUBLE: return jsmat<double>::Instantiate(hdd[0],hdd[1]);
      case TFLOAT: return jsmat<float>::Instantiate(hdd[0],hdd[1]);
      default:
	MERROR << "Cannot find suitable jsmatrix type for fits type " << _ftype << endl;
      };
      
      return Handle<Value>(Undefined());
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
