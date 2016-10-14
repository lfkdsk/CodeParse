#ifndef PRIVATE_SHARE_H_INCLUDED
#define PRIVATE_SHARE_H_INCLUDED
#include "DVM_code.h"

typedef struct {
    char        *mnemonic;
    char        *parameter;
    int         stack_increment;
} OpcodeInfo;

/* dispose.c */
void dvm_dispose_executable(DVM_Executable *exe);
/* wchar.c */
size_t dvm_wcslen(wchar_t *str);
wchar_t *dvm_wcscpy(wchar_t *dest, wchar_t *src);
wchar_t *dvm_wcsncpy(wchar_t *dest, wchar_t *src, size_t n);
int dvm_wcscmp(wchar_t *s1, wchar_t *s2);
wchar_t *dvm_wcscat(wchar_t *s1, wchar_t *s2);
int dvm_mbstowcs_len(const char *src);
void dvm_mbstowcs(const char *src, wchar_t *dest);
int dvm_wcstombs_len(const wchar_t *src);
void dvm_wcstombs(const wchar_t *src, char *dest);
char *dvm_wcstombs_alloc(const wchar_t *src);
char dvm_wctochar(wchar_t src);
int dvm_print_wcs(FILE *fp, wchar_t *str);
int dvm_print_wcs_ln(FILE *fp, wchar_t *str);
DVM_Boolean dvm_iswdigit(wchar_t ch);
/* disassemble.c */
void dvm_disassemble(DVM_Executable *exe);

#endif  /* PRIVATE_SHARE_H_INCLUDED */

