/*
 string_pgmspace.c - string functions that support PROGMEM

 Modified from original source by Earle F. Philhower, III
 Original authorship:
 Copyright (c) 2015 Michael C. Miller.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// TODO - Optimize these routines to use 32-bit accesses whenever possible

#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <sys/pgmspace.h>

size_t strnlen_P(PGM_P s, size_t size)
{
    const char *cp = s;
    const uint32_t *pmem;
    char c = 0;

    // Take care of any misaligned starting data
    while ( (size > 0) && ((uint32_t)cp & 0x3) ) {
        c = pgm_read_byte(cp);
        if (!c) goto done;
        size--;
        cp++;
    }

    // We didn't find the end in the initial misaligned bits
    // Now try it 32-bits at a time while possible
    pmem = (const uint32_t*)cp;
    while (size > 3) {
      uint32_t w = *pmem;
      if (0 == (w & 0xff)) {
        cp = (const char *)pmem;
        goto done;
      }
      w = w >> 8;
      if (0 == (w & 0xff)) {
        cp = (const char *)pmem + 1;
        goto done;
      }
      w = w >> 8;
      if (0 == (w & 0xff)) {
        cp = (const char *)pmem + 2;
        goto done;
      }
      w = w >> 8;
      if (0 == (w & 0xff)) {
        cp = (const char *)pmem + 3;
        goto done;
      }
      pmem++;
      size -= 4;
    }

    // Take care of any straggling bytes
    cp = (const char *)pmem;
    while ( size > 0 ) {
        c = pgm_read_byte(cp);
        if (!c) goto done;
        size--;
        cp++;
    }

done:
    return (size_t) (cp - s);
}

char* strstr_P(const char* haystack, PGM_P needle)
{
    const char* pn = (const char*)(needle);
    if (haystack[0] == 0) {
        if (pgm_read_byte(pn)) {
	        return NULL;
        }
        return (char*) haystack;
    }

    while (*haystack) {
        size_t i = 0;
        while (true) {
            char n = pgm_read_byte(pn + i);
            if (n == 0) {
                return (char *) haystack;
            }
            if (n != haystack[i]) {
                break;
            }
            ++i;
        }
        ++haystack;
    }
    return NULL;
}

void* memcpy_P(void* dest, PGM_VOID_P src, size_t count)
{
    const uint8_t* read = (const uint8_t*)(src);
    uint8_t* write = (uint8_t*)(dest);

    // Optimize for the case when dest and src start at 4-byte alignment
    // In this case we can copy ~8x faster by simply reading and writing
    // 32-bit values until there's less than a whole word left to write
    if ( 0 == (((uint32_t)dest|(uint32_t)src) & 0x3) ) {
        const uint32_t* readW = (const uint32_t*)(src);
        uint32_t* writeW = (uint32_t*)(dest);
        while (count >= 4) {
            *writeW++ = *readW++;
            count -= 4;
        }
        // Let default byte-by-byte finish the work
        write = (uint8_t *) writeW;
        read = (const uint8_t*) readW;
    }

    while (count)
    {
        *write++ = pgm_read_byte(read++);
        count--;
    }

    return dest;
}

int memcmp_P(const void* buf1, PGM_VOID_P buf2P, size_t size)
{
    int result = 0;
    const uint8_t* read1 = (const uint8_t*)buf1;
    const uint8_t* read2 = (const uint8_t*)buf2P;

    while (size > 0) {
        uint8_t ch2 = pgm_read_byte(read2);
        uint8_t ch1 = *read1;
        if (ch1 != ch2) {
            result = (int)(ch1)-(int)(ch2);
            break;
        }

        read1++;
        read2++;
        size--;
    }

    return result;
}

void* memccpy_P(void* dest, PGM_VOID_P src, int c, size_t count)
{
    uint8_t* read = (uint8_t*)src;
    uint8_t* write = (uint8_t*)dest;
    void* result = NULL;

    while (count > 0) {
        uint8_t ch = pgm_read_byte(read++);
        *write++ = ch;
        count--;
        if (c == ch) {
            return write; // the value after the found c
        }
    }

    return result;
}

void *memmove_P(void *dest, const void *src, size_t n)
{
    if ((const char *)src >= (const char *)0x40000000)
        return memcpy_P(dest, src, n);
    else
        return memmove(dest, src, n);
}


void* memmem_P(const void* buf, size_t bufSize, PGM_VOID_P findP, size_t findPSize)
{
    const uint8_t* read = (const uint8_t*)buf;
    const uint8_t* find = (uint8_t*)findP;
    uint8_t first = pgm_read_byte(find++);

    findPSize--;

    while (bufSize > 0) {
        if (*read == first) {
            size_t findSize = findPSize;
            const uint8_t* tag = read + 1;
            size_t tagBufSize = bufSize - 1;
            const uint8_t* findTag = find;

            while (tagBufSize > 0 && findSize > 0) {
                uint8_t ch = pgm_read_byte(findTag++);
                if (ch != *tag) {
                    bufSize--;
                    read++;
                    break;
                }
                findSize--;
                tagBufSize--;
                tag++;
            }
            if (findSize == 0) {
                return (void*)read;
            }
        }
        else {
            bufSize--;
            read++;
        }
    }
    return NULL;
}

char* strncpy_P(char* dest, PGM_P src, size_t size)
{
    bool size_known = (size != SIZE_IRRELEVANT);
    const char* read = src;
    char* write = dest;
    char ch = '.';

    // Optimize for the case when the src starts at 4-byte alignment
    // In this case we can copy ~4x faster by simply reading and writing
    // 32-bit values until there's less than a whole word left to write
    if (!((((uint32_t)src)|(uint32_t)dest) & 0x3)) {
        while (size >= 4) {
            uint32_t p = *(uint32_t*)read;
            // Bit of magic to check if any bytes are 0 in the word, adapted from:
            // https://jameshfisher.com/2017/01/24/bitwise-check-for-zero-byte.html
            int hasZero = (p - 0x01010101) & ~p & 0x80808080;
            if (hasZero) {
                // Don't handle the partial word case here, use standard flow
                break;
            } else {
                *(uint32_t *)write = p;
                read += 4;
                write += 4;
                size -= 4;
            }
        }
    }

    while (size > 0 && ch != '\0')
    {
        ch = pgm_read_byte(read++);
        *write++ = ch;
        size--;
    }
    if (size_known)
    {
        while (size > 0)
        {
            *write++ = 0;
            size--;
        }
    }

    return dest;
}

char* strncat_P(char* dest, PGM_P src, size_t size)
{
    char* write = dest;

    while (*write != '\0')
    {
        write++;
    }

    const char* read = src;
    char ch = '.';

    while (size > 0 && ch != '\0')
    {
        ch = pgm_read_byte(read++);
        *write++ = ch;

        size--;
    }

    if (ch != '\0')
    {
        *write = '\0';
    }

    return dest;
}

int strncmp_P(const char* str1, PGM_P str2P, size_t size)
{
    int result = 0;

    while (size > 0)
    {
        char ch1 = *str1++;
        char ch2 = pgm_read_byte(str2P++);
        result = ch1 - ch2;
        if (result != 0 || ch2 == '\0')
        {
            break;
        }

        size--;
    }

    return result;
}

int strncasecmp_P(const char* str1, PGM_P str2P, size_t size)
{
    int result = 0;

    while (size > 0)
    {
        char ch1 = tolower(*str1++);
        char ch2 = tolower(pgm_read_byte(str2P++));
        result = ch1 - ch2;
        if (result != 0 || ch2 == '\0')
        {
            break;
        }

        size--;
    }

    return result;
}

void *memchr_P(const void *src_void, int c, size_t length)
{
    const unsigned char *src = (const unsigned char *) src_void;
    unsigned char d = c;
    while (length--)
    {
        if (pgm_read_byte(src) == d) {
            return (void *) src;
        }
        src++;
    }

    return NULL;
}

