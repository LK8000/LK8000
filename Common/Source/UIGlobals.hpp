/* 
 * File:   UIGlobals.hpp
 * Author: user
 *
 * Created on May 15, 2015
 */

#ifndef UIGLOBALS_HPP
#define	UIGLOBALS_HPP

#include "Compiler.h"

class SingleWindow;

namespace UIGlobals {
    
#ifndef WIN32    
  gcc_const
  SingleWindow &GetMainWindow();
#endif
  
};

#endif	/* UIGLOBALS_HPP */

