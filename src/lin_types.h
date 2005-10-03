#ifndef __LIN_TYPES_H
#define __LIN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//Type definitions for all linearisers

typedef enum { lmStack, lmRegular, lmRegular2, lmAlternative } t_lin_method;
//t_lin_method represents the available linearisation methods

typedef enum { cmFull, cmNone } t_cluster_method;
//t_cluster_method represents the available clustering methods

typedef struct {
  t_lin_method lin_method;
  t_cluster_method intermediate_cluster_method;
  t_cluster_method final_cluster_method;
  bool newstate;
  bool binary;
  bool statenames;
  bool norewrite;
  bool nofreevars;
} t_lin_options;
//t_lin_options represents the options of the lineariser

#ifdef __cplusplus
}
#endif

#endif //__LIN_TYPES_H
