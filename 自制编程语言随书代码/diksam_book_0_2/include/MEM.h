#ifndef PUBLIC_MEM_H
#define PUBLIC_MEM_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    MEM_FAIL_AND_EXIT,
    MEM_FAIL_AND_RETURN
} MEM_FailMode;

typedef struct MEM_Controller_tag *MEM_Controller;
typedef void (*MEM_ErrorHandler)(MEM_Controller, char *, int, char *);
typedef struct MEM_Storage_tag *MEM_Storage;

extern MEM_Controller mem_default_controller;

#ifdef MEM_CONTROLLER
#define MEM_CURRENT_CONTROLLER MEM_CONTROLLER
#else /* MEM_CONTROLLER */
#define MEM_CURRENT_CONTROLLER mem_default_controller
#endif /* MEM_CONTROLLER */

/*
 * Don't use mem_*_func function.
 * There are private functions of MEM module.
 */
MEM_Controller MEM_create_controller(void);
void *MEM_malloc_func(MEM_Controller controller,
                      char *filename, int line, size_t size);
void *MEM_realloc_func(MEM_Controller controller,
                       char *filename, int line, void *ptr, size_t size);
char *MEM_strdup_func(MEM_Controller controller,
                      char *filename, int line, char *str);
MEM_Storage MEM_open_storage_func(MEM_Controller controller,
                                  char *filename, int line, int page_size);
void *MEM_storage_malloc_func(MEM_Controller controller,
                              char *filename, int line,
                              MEM_Storage storage, size_t size);
void MEM_free_func(MEM_Controller controller, void *ptr);
void MEM_dispose_storage_func(MEM_Controller controller,
                              MEM_Storage storage);

void MEM_set_error_handler(MEM_Controller controller,
                           MEM_ErrorHandler handler);
void MEM_set_fail_mode(MEM_Controller controller,
                       MEM_FailMode mode);
void MEM_dump_blocks_func(MEM_Controller controller, FILE *fp);
void MEM_check_block_func(MEM_Controller controller,
                          char *filename, int line, void *p);
void MEM_check_all_blocks_func(MEM_Controller controller,
                               char *filename, int line);

#define MEM_malloc(size)\
  (MEM_malloc_func(MEM_CURRENT_CONTROLLER,\
                   __FILE__, __LINE__, size))
#define MEM_realloc(ptr, size)\
  (MEM_realloc_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, ptr, size))
#define MEM_strdup(str)\
  (MEM_strdup_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, str))
#define MEM_open_storage(page_size)\
  (MEM_open_storage_func(MEM_CURRENT_CONTROLLER,\
                         __FILE__, __LINE__, page_size))
#define MEM_storage_malloc(storage, size)\
  (MEM_storage_malloc_func(MEM_CURRENT_CONTROLLER,\
                           __FILE__, __LINE__, storage, size))
#define MEM_free(ptr) (MEM_free_func(MEM_CURRENT_CONTROLLER, ptr))
#define MEM_dispose_storage(storage)\
  (MEM_dispose_storage_func(MEM_CURRENT_CONTROLLER, storage))
#ifdef DEBUG
#define MEM_dump_blocks(fp)\
  (MEM_dump_blocks_func(MEM_CURRENT_CONTROLLER, fp))
#define MEM_check_block(p)\
  (MEM_check_block_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, p))
#define MEM_check_all_blocks()\
  (MEM_check_all_blocks_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__))
#else /* DEBUG */
#define MEM_dump_blocks(fp) ((void)0)
#define MEM_check_block(p)  ((void)0)
#define MEM_check_all_blocks() ((void)0)
#endif /* DEBUG */

#endif  /* PUBLIC_MEM_H */
