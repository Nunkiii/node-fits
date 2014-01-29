//The Quarklib project (PG Sprimont<fullmoon@swing.be>)

#ifndef __ANSI_H__
#define __ANSI_H__

#include <iostream>


#define TBOLD "\x1B[1m"
#define REV  "\x1B[7m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"
#define CYAN  "\x1B[36m"
#define YELLOW  "\x1B[33m"
#define MAGENTA "\x1B[35m"
#define BEEP  "\x7"
#define BGBLUE  "\x1B[44m"
#define NORMAL "\x1B[0m"
#define CLEANS  "\x1B[2J"


#define GENERAL_DEBUG

#define LOC string(__LINE__) + " : " + string(__FILE__) 

#ifdef GENERAL_DEBUG

#define MERROR std::cerr << RED <<   "\r(E)\t"<< GREEN << __FILE__ << "," << __LINE__ << NORMAL << "\t" <<TBOLD<<__FUNCTION__<<NORMAL << "\t" 
#define MINFO std::cout << BLUE <<   "\r(I)\t"<< GREEN << __FILE__ << "," << __LINE__ << NORMAL << "\t" <<TBOLD<<__FUNCTION__<<NORMAL << "\t" 
#define MWARN std::cout << YELLOW << "\r(W)\t"<< GREEN << __FILE__ << "," << __LINE__ << NORMAL << "\t" <<TBOLD<<__FUNCTION__<<NORMAL << "\t" 
#else
#define MERROR std::cerr<< TBOLD<< RED <<__FUNCTION__<<NORMAL << "\t"
#define MINFO  std::cout << TBOLD<< GREEN <<__FUNCTION__<<NORMAL << "\t"
#define MWARN  std::cout << TBOLD<< YELLOW <<__FUNCTION__<<NORMAL << "\t"
#endif


#endif
