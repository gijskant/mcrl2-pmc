#ifndef UI_CORE_H
#define UI_CORE_H

#include "settings_manager.h"
#include "tool_manager.h"

namespace squadt {

  /* Global Settings Manager component */
  extern settings_manager::ptr global_settings_manager;

  /* Global Tool Manager component */
  extern tool_manager::ptr     global_tool_manager;
}

#endif

