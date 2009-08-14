/*
 * $Id: helper.cpp,v 1.3 2005/10/18 11:30:19 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
  * based on nhttpd (C) 2001/2002 Dirk Szymanski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef ENABLE_EXPERT_WEBIF
#include <stdlib.h>                      // calloc and free prototypes.
#include <stdio.h>                       // printf prototype.
#include <string.h>                      // str* and memset prototypes.

#include "helper.h"

string itoh(unsigned int conv)
{
	static char buf[20];
	sprintf(buf,"0x%06x",conv);
	return string(buf);
}

string itoa(unsigned int conv)
{
	static char buf[20];
	sprintf(buf,"%u",conv);
	return string(buf);
}

typedef unsigned char uchar;
typedef unsigned int uint;

bool    b64valid(uchar *);                  // Tests for a valid Base64 char.
string  b64isnot(char *, char *);           // Displays an invalid message.
char    *b64buffer(char *, bool);            // Alloc. encoding/decoding buffer.

// Macro definitions:

#define b64is7bit(c)  ((c) > 0x7f ? 0 : 1)  // Valid 7-Bit ASCII character?
#define b64blocks(l) (((l) + 2) / 3 * 4 + 1)// Length rounded to 4 byte block.
#define b64octets(l)  ((l) / 4  * 3 + 1)    // Length rounded to 3 byte octet.

// Note:    Tables are in hex to support different collating sequences

static  const                               // Base64 Index into encoding
uchar  pIndex[]     =   {                   // and decoding table.
                        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
                        0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
                        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
                        0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
                        0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
                        0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
                        0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33,
                        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2b, 0x2f
                        };

static const                                // Base64 encoding and decoding
uchar   pBase64[]   =   {                   // table.
                        0x3e, 0x7f, 0x7f, 0x7f, 0x3f, 0x34, 0x35, 0x36,
                        0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x7f,
                        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x01,
                        0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
                        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x1a, 0x1b,
                        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
                        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
                        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
                        };

string b64decode(char *s)
{
    int     l = strlen(s);                  // Get length of Base64 string.
    char   *b = 0, *p = 0;                  // Decoding buffer pointers.
    uchar   c = 0;                          // Character to decode.
    int     x = 0;                          // General purpose integers.
    int     y = 0;

    static                                  // Collating sequence...
    const                                   // ...independant "===".
    char    pPad[]  =   {0x3d, 0x3d, 0x3d, 0x00};

    if  (l % 4)                             // If it's not modulo 4, then it...
        return b64isnot(s, NULL);           // ...can't be a Base64 string.

    if  (b == strchr(s, pPad[0]))           // Only one, two or three equal...
    {                                       // ...'=' signs are allowed at...
        if  ((b - s) < (l - 3))             // ...the end of the Base64 string.
            return b64isnot(s, NULL);       // Any other equal '=' signs are...
        else                                // ...invalid.
            if  (strncmp(b, (char *) pPad + 3 - (s + l - b), s + l - b))
                return b64isnot(s, NULL);
    }

    if  (!(b = b64buffer(s, false)))        // Allocate a decoding buffer.
        return false;                       // Can't allocate decoding buffer.

    p = s;                                  // Save the encoded string pointer.
    x = 0;                                  // Initialize index.

    while ((c = *s++))                      // Decode every byte of the...
    {                                       // Base64 string.
        if  (c == pPad[0])                  // Ignore "=".
            break;

        if (!b64valid(&c))                  // Valid Base64 Index?
            return b64isnot(s, b);          // No, return false.
        
        switch(x % 4)                       // Decode 4 byte words into...
        {                                   // ...3 byte octets.
        case    0:                          // Byte 0 of word.
            b[y]    =  c << 2;
            break;                          
        case    1:                          // Byte 1 of word.
            b[y]   |=  c >> 4;

            if (!b64is7bit((uchar) b[y++])) // Is 1st byte of octet valid?
                return b64isnot(s, b);      // No, return false.

            b[y]    = (c & 0x0f) << 4;
            break;
        case    2:                          // Byte 2 of word.
            b[y]   |=  c >> 2;

            if (!b64is7bit((uchar) b[y++])) // Is 2nd byte of octet valid?
                return b64isnot(s, b);      // No, return false.

            b[y]    = (c & 0x03) << 6;
            break;
        case    3:                          // Byte 3 of word.
            b[y]   |=  c;

            if (!b64is7bit((uchar) b[y++])) // Is 3rd byte of octet valid?
                return b64isnot(s, b);      // No, return false.
        }
        x++;                                // Increment word byte.
    }

	string b_str = string(b);
    free(b);                                // De-allocate decoding buffer.
    return b_str;                            // Return to caller with success.
}

bool b64valid(uchar *c)
{
    if ((*c < 0x2b) || (*c > 0x7a))         // If not within the range of...
        return false;                       // ...the table, return false.
    
    if ((*c = pBase64[*c - 0x2b]) == 0x7f)  // If it falls within one of...
        return false;                       // ...the gaps, return false.

    return true;                            // Otherwise, return true.
}

string b64isnot(char *p, char *b)
{
    printf("\"%s\" is not a Base64 encoded string.\n", p);

    if  (b)                                 // If the buffer pointer is not...
        free(b);                            // ...NULL, de-allocate it.

    return  "";                          // Return false for main.
}

char *b64buffer(char *s, bool f)
{
    int     l = strlen(s);                  // String size to encode or decode.
    char   *b;                              // String pointers.

    if  (!l)                                // If the string size is 0...
        return  NULL;                       // ...return null.

   if (!(b = (char *) calloc((f ? b64blocks(l) : b64octets(l)),
               sizeof(char))))
        printf("Insufficient real memory to %s \"%s\".\n",
              (f ? "encode" : "decode"), s);
    return  b;                              // Return the pointer or null.
}
#endif

