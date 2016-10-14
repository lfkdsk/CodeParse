#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"

static void default_error_handler(MEM_Controller controller,
                                  char *filename, int line, char *msg);

static struct MEM_Controller_tag st_default_controller = {
    NULL,/* stderr */
    default_error_handler,
    MEM_FAIL_AND_EXIT
};
MEM_Controller mem_default_controller = &st_default_controller;

typedef union {
    long        l_dummy;
    double      d_dummy;
    void        *p_dummy;
} Align;

#define MARK_SIZE       (4)

typedef struct {
    int         size;
    char        *filename;
    int         line;
    Header      *prev;
    Header      *next;
    unsigned char       mark[MARK_SIZE];
} HeaderStruct;

#define ALIGN_SIZE      (sizeof(Align))
#define revalue_up_align(val)   ((val) ? (((val) - 1) / ALIGN_SIZE + 1) : 0)
#define HEADER_ALIGN_SIZE       (revalue_up_align(sizeof(HeaderStruct)))
#define MARK (0xCD)

union Header_tag {
    HeaderStruct        s;
    Align               u[HEADER_ALIGN_SIZE];
};

static void
default_error_handler(MEM_Controller controller,
                      char *filename, int line, char *msg)
{
    fprintf(controller->error_fp,
            "MEM:%s failed in %s at %d\n", msg, filename, line);
}

static void
error_handler(MEM_Controller controller, char *filename, int line, char *msg)
{
    if (controller->error_fp == NULL) {
        controller->error_fp = stderr;
    }
    controller->error_handler(controller, filename, line, msg);

    if (controller->fail_mode == MEM_FAIL_AND_EXIT) {
        exit(1);
    }
}

MEM_Controller
MEM_create_controller(void)
{
    MEM_Controller      p;

    p = MEM_malloc_func(&st_default_controller, __FILE__, __LINE__,
                        sizeof(struct MEM_Controller_tag));
    *p = st_default_controller;

    return p;
}

#ifdef DEBUG
static void
chain_block(MEM_Controller controller, Header *new_header)
{
    if (controller->block_header) {
        controller->block_header->s.prev = new_header;
    }
    new_header->s.prev = NULL;
    new_header->s.next = controller->block_header;
    controller->block_header = new_header;
}

static void
rechain_block(MEM_Controller controller, Header *header)
{
    if (header->s.prev) {
        header->s.prev->s.next = header;
    } else {
        controller->block_header = header;
    }
    if (header->s.next) {
        header->s.next->s.prev = header;
    }
}

static void
unchain_block(MEM_Controller controller, Header *header)
{
    if (header->s.prev) {
        header->s.prev->s.next = header->s.next;
    } else {
        controller->block_header = header->s.next;
    }
    if (header->s.next) {
        header->s.next->s.prev = header->s.prev;
    }
}

void
set_header(Header *header, int size, char *filename, int line)
{
    header->s.size = size;
    header->s.filename = filename;
    header->s.line = line;
    memset(header->s.mark, MARK, (char*)&header[1] - (char*)header->s.mark);
}

void
set_tail(void *ptr, int alloc_size)
{
    char *tail;
    tail = ((char*)ptr) + alloc_size - MARK_SIZE;
    memset(tail, MARK, MARK_SIZE);
}

void
check_mark_sub(unsigned char *mark, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (mark[i] != MARK) {
            fprintf(stderr, "bad mark\n");
            abort();
        }
    }
}

void
check_mark(Header *header)
{
    unsigned char        *tail;
    check_mark_sub(header->s.mark, (char*)&header[1] - (char*)header->s.mark);
    tail = ((unsigned char*)header) + header->s.size + sizeof(Header);
    check_mark_sub(tail, MARK_SIZE);
}
#endif /* DEBUG */

void*
MEM_malloc_func(MEM_Controller controller, char *filename, int line,
                size_t size)
{
    void        *ptr;
    size_t      alloc_size;

#ifdef DEBUG
    alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    alloc_size = size;
#endif
    ptr = malloc(alloc_size);
    if (ptr == NULL) {
        error_handler(controller, filename, line, "malloc");
    }

#ifdef DEBUG
    memset(ptr, 0xCC, alloc_size);
    set_header(ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header*)ptr);
    ptr = (char*)ptr + sizeof(Header);
#endif

    return ptr;
}

void*
MEM_realloc_func(MEM_Controller controller, char *filename, int line,
                 void *ptr, size_t size)
{
    void        *new_ptr;
    size_t      alloc_size;
    void        *real_ptr;
#ifdef DEBUG
    Header      old_header;
    int         old_size;

    alloc_size = size + sizeof(Header) + MARK_SIZE;
    if (ptr != NULL) {
        real_ptr = (char*)ptr - sizeof(Header);
        check_mark((Header*)real_ptr);
        old_header = *((Header*)real_ptr);
        old_size = old_header.s.size;
        unchain_block(controller, real_ptr);
    } else {
        real_ptr = NULL;
        old_size = 0;
    }
#else
    alloc_size = size;
    real_ptr = ptr;
#endif

    new_ptr = realloc(real_ptr, alloc_size);
    if (new_ptr == NULL) {
        if (ptr == NULL) {
            error_handler(controller, filename, line, "realloc(malloc)");
        } else {
            error_handler(controller, filename, line, "realloc");
            free(real_ptr);
        }
    }

#ifdef DEBUG
    if (ptr) {
        *((Header*)new_ptr) = old_header;
        ((Header*)new_ptr)->s.size = size;
        rechain_block(controller, (Header*)new_ptr);
        set_tail(new_ptr, alloc_size);
    } else {
        set_header(new_ptr, size, filename, line);
        set_tail(new_ptr, alloc_size);
        chain_block(controller, (Header*)new_ptr);
   }
    new_ptr = (char*)new_ptr + sizeof(Header);
    if (size > old_size) {
        memset((char*)new_ptr + old_size, 0xCC, size - old_size);
    }
#endif

    return(new_ptr);
}

char *
MEM_strdup_func(MEM_Controller controller, char *filename, int line,
                char *str)
{
    char        *ptr;
    int         size;
    size_t      alloc_size;

    size = strlen(str) + 1;
#ifdef DEBUG
    alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    alloc_size = size;
#endif
    ptr = malloc(alloc_size);
    if (ptr == NULL) {
        error_handler(controller, filename, line, "strdup");
    }

#ifdef DEBUG
    memset(ptr, 0xCC, alloc_size);
    set_header((Header*)ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header*)ptr);
    ptr = (char*)ptr + sizeof(Header);
#endif
    strcpy(ptr, str);

    return(ptr);
}

void
MEM_free_func(MEM_Controller controller, void *ptr)
{
    void        *real_ptr;
#ifdef DEBUG
    int size;
#endif
    if (ptr == NULL)
        return;

#ifdef DEBUG
    real_ptr = (char*)ptr - sizeof(Header);
    check_mark((Header*)real_ptr);
    size = ((Header*)real_ptr)->s.size;
    unchain_block(controller, real_ptr);
    memset(real_ptr, 0xCC, size + sizeof(Header));
#else
    real_ptr = ptr;
#endif

    free(real_ptr);
}

void
MEM_set_error_handler(MEM_Controller controller, MEM_ErrorHandler handler)
{
    controller->error_handler = handler;
}

void
MEM_set_fail_mode(MEM_Controller controller, MEM_FailMode mode)
{
    controller->fail_mode = mode;
}

void
MEM_dump_blocks_func(MEM_Controller controller, FILE *fp)
{
#ifdef DEBUG
    Header *pos;
    int counter = 0;
    int i;

    for (pos = controller->block_header; pos; pos = pos->s.next) {
        check_mark(pos);
        fprintf(fp, "[%04d]%p********************\n", counter,
                (char*)pos + sizeof(Header));
        fprintf(fp, "%s line %d size..%d\n",
                pos->s.filename, pos->s.line, pos->s.size);
        fprintf(fp, "[");
        for (i = 0; i < pos->s.size; i++) {
            if (isprint(*((char*)pos + sizeof(Header)+i))) {
                fprintf(fp, "%c", *((char*)pos + sizeof(Header)+i));
            } else {
                fprintf(fp, ".");
            }
        }
        fprintf(fp, "]\n");
        counter++;
    }
#endif /* DEBUG */
}

void
MEM_check_block_func(MEM_Controller controller, char *filename, int line,
                     void *p)
{
#ifdef DEBUG
    void *real_ptr = ((char*)p) - sizeof(Header);

    check_mark(real_ptr);
#endif /* DEBUG */
}

void MEM_check_all_blocks_func(MEM_Controller controller,
                               char *filename, int line)
{
#ifdef DEBUG
    Header *pos;

    for (pos = controller->block_header; pos; pos = pos->s.next) {
        check_mark(pos);
    }
#endif /* DEBUG */
}
