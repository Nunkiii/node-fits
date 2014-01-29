#include "threads.hh"
#include "ansi.h"
#include <string.h>
#include <errno.h>
#include <iostream>


namespace qk{

  using namespace std;

  void * thread_run(void* _in){
    thread * tsk = (thread*) _in;
    tsk->exec();
    return (void*) _in;
  }

  cond::cond(){
    pthread_cond_init(&cnd,NULL);
    cnt=0;
  }
  
//   bool cond::bcast(){
//     //  qkmsg << "cond: locking MUT" << endl;
//     pthread_mutex_lock(&mut);
//     //  qkmsg << "cond: BCAST" << endl;
//     cnt++;
//     pthread_cond_broadcast(&cnd);
//     //  qkmsg << "cond: unlocking MUT" << endl;
//     pthread_mutex_unlock(&mut);  
//     return true;
//   }
  
  thread::thread(){
  }
  
  thread::~thread(){
    //  cout << "\rThread dying.." << endl;
  }

  bool thread::exec(){return true;}

  bool thread::start(){

    //MINFO << "Creating thread ..." << endl;

    if(pthread_create(&th,NULL,thread_run, this)!=0){
      MERROR << "thread: pthread_create:  " 
	   << strerror(errno) << endl;
      return false;
    }    
    
    return true;
  }


}
