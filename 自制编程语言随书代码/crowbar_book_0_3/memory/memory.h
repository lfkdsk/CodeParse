#ifndef PRIVATE_MEM_H_INCLUDED
#define PRIVATE_MEM_H_INCLUDED
#include "MEM.h"

typedef union Header_tag Header;


struct MEM_Controller_tag {
    FILE        *error_fp;
    MEM_ErrorHandler    error_handler;
    MEM_FailMode        fail_mode;
    Header      *block_header;
};
#endif /* PRIVATE_MEM_H_INCLUDED */

