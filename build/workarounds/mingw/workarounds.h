#ifndef _MINGW_SUBSTITUTES_H_
#define _MINGW_SUBSTITUTES_H_

/*
 * Windows MinGW Compatibility Functions -- functions to support portability to the
 * Windows environment.
 */

#include <cassert>
#include <cerrno>
#include <process.h>
#include <unistd.h>
#include <windows.h>

#define getpid   _getpid

inline pid_t waitpid(pid_t pid, int* status, int options) {

  return _cwait (status, pid, _WAIT_CHILD);
}

inline int kill(int pid, int signal) {
  return (TerminateProcess(reinterpret_cast < void* > (pid), signal));
}

#define SIGKILL 9

#ifndef WIFEXITED
  #define WIFEXITED(S) 1
#endif

#ifndef WEXITSTATUS
  #define WEXITSTATUS(S) (S)
#endif

#include "../workarounds.h"

#endif
