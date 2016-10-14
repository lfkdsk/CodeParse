#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "DBG.h"
#include "MEM.h"
#include "DVM.h"

size_t
dvm_wcslen(wchar_t *str)
{
    return wcslen(str);
}

wchar_t *
dvm_wcscpy(wchar_t *dest, wchar_t *src)
{
    return wcscpy(dest, src);
}

wchar_t *
dvm_wcsncpy(wchar_t *dest, wchar_t *src, size_t n)
{
    return wcsncpy(dest, src, n);
}

int
dvm_wcscmp(wchar_t *s1, wchar_t *s2)
{
    return wcscmp(s1, s2);
}

wchar_t *
dvm_wcscat(wchar_t *s1, wchar_t *s2)
{
    return wcscat(s1, s2);
}

int
dvm_mbstowcs_len(const char *src)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
        if (status < 0) {
            return status;
        }
        dest_idx++;
        src_idx += status;
    }

    return dest_idx;
}

void
dvm_mbstowcs(const char *src, wchar_t *dest)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(&dest[dest_idx], &src[src_idx],
                         MB_LEN_MAX, &ps);
        dest_idx++;
        src_idx += status;
    }
    dest[dest_idx] = L'\0';
}

int
dvm_wcstombs_len(const wchar_t *src)
{
    int src_idx, dest_idx;
    int status;
    char dummy[MB_LEN_MAX];
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != L'\0'; ) {
        status = wcrtomb(dummy, src[src_idx], &ps);
        src_idx++;
        dest_idx += status;
    }

    return dest_idx;
}

void
dvm_wcstombs(const wchar_t *src, char *dest)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = wcrtomb(&dest[dest_idx], src[src_idx], &ps);
        src_idx++;
        dest_idx += status;
    }
    dest[dest_idx] = '\0';
}

char *
dvm_wcstombs_alloc(const wchar_t *src)
{
    int len;
    char *ret;

    len = dvm_wcstombs_len(src);
    ret = MEM_malloc(len + 1);
    dvm_wcstombs(src, ret);

    return ret;
}

char
dvm_wctochar(wchar_t src)
{
    mbstate_t ps;
    int status;
    char dest;

    memset(&ps, 0, sizeof(mbstate_t));
    status = wcrtomb(&dest, src, &ps);
    DBG_assert(status == 1, ("wcrtomb status..%d\n", status));

    return dest;
}

int
dvm_print_wcs(FILE *fp, wchar_t *str)
{
    char *tmp;
    int mb_len;
    int result;

    mb_len = dvm_wcstombs_len(str);
    MEM_check_all_blocks();
    tmp = MEM_malloc(mb_len + 1);
    dvm_wcstombs(str, tmp);
    result = fprintf(fp, "%s", tmp);
    MEM_free(tmp);

    return result;
}


int
dvm_print_wcs_ln(FILE *fp, wchar_t *str)
{
    int result;

    result = dvm_print_wcs(fp, str);
    fprintf(fp, "\n");

    return result;
}

DVM_Boolean
dvm_iswdigit(wchar_t ch)
{
    return ch >= L'0' && ch <= L'9';
}
