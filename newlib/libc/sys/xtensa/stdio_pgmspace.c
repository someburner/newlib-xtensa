#include <stdio.h>
#include <sys/stdio.h>

/* The following functions are now effectively no-ops and call the normal
   STDIO function because it's been modified to support PROGMEM automatically
*/

int printf_P(PGM_P formatP, ...) {
    int ret;
    va_list arglist;
    va_start(arglist, formatP);
    ret = vprintf(formatP, arglist);
    va_end(arglist);
    return ret;
}

int sprintf_P(char* str, PGM_P formatP, ...) {
    int ret;
    va_list arglist;
    va_start(arglist, formatP);
    ret = vsprintf(str, formatP, arglist);
    va_end(arglist);
    return ret;
}

int snprintf_P(char* str, size_t strSize, PGM_P formatP, ...) {
    int ret;
    va_list arglist;
    va_start(arglist, formatP);
    ret = vsnprintf(str, strSize, formatP, arglist);
    va_end(arglist);
    return ret;
}


int vsnprintf_P(char* str, size_t strSize, PGM_P formatP, va_list ap) {
    int ret;
    ret = vsnprintf(str, strSize, formatP, ap);
    return ret;
}

