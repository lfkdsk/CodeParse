#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "DBG.h"
#include "crowbar.h"

size_t
CRB_wcslen(CRB_Char *str)
{
    return wcslen(str);
}

CRB_Char *
CRB_wcscpy(CRB_Char *dest, CRB_Char *src)
{
    return wcscpy(dest, src);
}

CRB_Char *
CRB_wcsncpy(CRB_Char *dest, CRB_Char *src, size_t n)
{
    return wcsncpy(dest, src, n);
}

int
CRB_wcscmp(CRB_Char *s1, CRB_Char *s2)
{
    return wcscmp(s1, s2);
}

CRB_Char *
CRB_wcscat(CRB_Char *s1, CRB_Char *s2)
{
    return wcscat(s1, s2);
}

int
CRB_mbstowcs_len(const char *src)
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
CRB_mbstowcs(const char *src, CRB_Char *dest)
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

CRB_Char *
CRB_mbstowcs_alloc(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   int line_number, const char *src)
{
    int len;
    CRB_Char *ret;

    len = CRB_mbstowcs_len(src);
    if (len < 0) {
        crb_runtime_error(line_number,
                          BAD_MULTIBYTE_CHARACTER_ERR,
                          MESSAGE_ARGUMENT_END);
        return NULL;
    }
    ret = MEM_malloc(sizeof(CRB_Char) * (len+1));
    CRB_mbstowcs(src, ret);

    return ret;
}

int
CRB_wcstombs_len(const CRB_Char *src)
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
CRB_wcstombs(const CRB_Char *src, char *dest)
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
CRB_wcstombs_alloc(const CRB_Char *src)
{
    int len;
    char *ret;

    len = CRB_wcstombs_len(src);
    ret = MEM_malloc(len + 1);
    CRB_wcstombs(src, ret);

    return ret;
}

char
CRB_wctochar(CRB_Char src)
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
CRB_print_wcs(FILE *fp, CRB_Char *str)
{
    char *tmp;
    int mb_len;
    int result;

    mb_len = CRB_wcstombs_len(str);
    tmp = MEM_malloc(mb_len + 1);
    CRB_wcstombs(str, tmp);
    result = fprintf(fp, "%s", tmp);
    MEM_free(tmp);

    return result;
}


int
CRB_print_wcs_ln(FILE *fp, CRB_Char *str)
{
    int result;

    result = CRB_print_wcs(fp, str);
    fprintf(fp, "\n");

    return result;
}

CRB_Boolean
CRB_iswdigit(CRB_Char ch)
{
    return ch >= L'0' && ch <= L'9';
}
