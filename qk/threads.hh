//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __THREAD_UTS_H__
#define __THREAD_UTS_H__

#include <pthread.h>

namespace qk{

  ///Simple mutex interface.

  class mutex{
  public:
    ///Constructor.
    mutex(){
      //we initialize the mutex with the default options.
      pthread_mutex_init(&mut,NULL); 
    }
    ///Desstructor. (Nada)

    virtual ~mutex(){
    }
 
    ///Locks/Unlocks the mutex.

    int lock(bool _dir=1, bool _try=0){
      if(_dir){
	if(_try)
	  return pthread_mutex_trylock(&mut)==0;      
	else return pthread_mutex_lock(&mut)==0;
      }
      
      return pthread_mutex_unlock(&mut)==0;            
    }
    ///Unlocks the mutex.
    int unlock(){return pthread_mutex_unlock(&mut);}
    ///Retrieves the mutex object.
    const pthread_mutex_t& get_mutex() const {return mut;}

  protected:
    ///The libpthread mutex object.
    pthread_mutex_t  mut;  
  }; 
  

  ///Condition object (and the associated mutex) for thread synchronization.
  ///Straightforward interface to the pthread_cond_t functions. 
  
  class cond : public mutex{
  public:
    cond();
    virtual ~cond(){}
    
    ///Broadcasts the condition
    int broadcast(){return pthread_cond_broadcast(&cnd);}
    ///Signals the condition
    int signal(){return pthread_cond_signal(&cnd);}
    ///Waits for the condition
    int wait(){return pthread_cond_wait(&cnd, &mut);}
    ///Waits for the condition with time limit given by _abstime.
    int twait(struct timespec& _abstime){return pthread_cond_timedwait(&cnd, &mut,&_abstime);}

  protected:
    
    int cnt;
    pthread_cond_t  cnd;  
  };

  ///Simple thread interface.

  class thread{
  public:
    thread();
    virtual ~thread();

    ///The main thread function.
    
    virtual bool exec();

    ///Starts the thread and calls the thread function.
    bool start();
    ///Retrieves the thread object.
    const pthread_t& get_thread() const { return th;}

    int join(void** _return_value){    
      return pthread_join(th,_return_value);
    }
    
  private:

    pthread_t th;
  };

}


#endif
