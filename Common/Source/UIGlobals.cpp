#include "UIGlobals.hpp"
#include "externs.h"

#ifndef WIN32
SingleWindow &
UIGlobals::GetMainWindow()
{
  return (*main_window);
}
#endif