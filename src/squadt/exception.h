#ifndef SQUADT_EXCEPTION_H
#define SQUADT_EXCEPTION_H

#include <exception/exception.h>

namespace squadt {
  namespace exception_identifier {

    enum values {
      cannot_access_user_settings_directory, ///< \brief the directory where user settings must be stored is not accessible
      failed_loading_object,                 ///< \brief failed to load information
      required_attributes_missing,           ///< \brief input from file failed due to errors in input
      requested_tool_unavailable,            ///< \brief requested tool is not available
      program_execution_failed               ///< \brief program execution failed
    };
  }

  typedef exception::exception < exception_identifier::values > exception;
}

#endif
