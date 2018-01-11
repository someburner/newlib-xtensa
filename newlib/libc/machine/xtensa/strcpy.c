/* strcpy.c - Xtensa code to determine if source is PMEM or RAM and call appropriate strcpy routine.

  GCC is a very smart compiler, and it will, in fact, replace printf(), and its
  related functions with strcpy() calls in order to optimize speed.  This fails
  horribly when printf(PSTR("xxx")) is called since the original strcpy can't
  handle the PROGMEM source.

  See http://www.ciselant.de/projects/gcc_printf/gcc_printf.html for more info.

  There are two ways around this:
  - Adding -fno-builtin-*printf, which slows down every printf() call that was
    being optimized before.  GCC won't replace the printf() call with a strcpy()
    call, and everything will work since we now support PROGMEM strings in
    printf().
  - Make strcpy smarter and fall back on the appropriate routine depending on
    the source parameter.  Since on the ESP8266 PROGMEM starts at 0x40000000
    this is a simple comparison.  In this case speed will be maintained.
*/

#include <string.h>

extern char *__fast_strcpy(char *dest, const char *src);
char *strcpy(char *dest, const char *src)
{
    if (src >= (const char *)0x40000000) return strcpy_P(dest, src);
    else return __fast_strcpy(dest, src);
}

extern char *__fast_strncpy(char *dest, const char *src, size_t n);
char *strncpy(char *dest, const char *src, size_t n)
{
    if (src >= (const char *)0x40000000) return strncpy_P(dest, src, n);
    else return __fast_strncpy(dest, src, n);
}
