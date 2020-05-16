/*============================================================ Rev. 15 Jul 1993
  Routines for TINY table driven CRC-16 & CRC-32, including building tables
  Refer to CRC.DOC for information and documentation.
  If CRC_TINY is defined for CRC.H, this file replaces CRC.C (or CRC.ASM)
  -----------------------------------------------------------------------------

               Information collected and edited by Arjen G. Lentz
             Sourcecode in C and 80x86 ASM written by Arjen G. Lentz
                  COPYRIGHT (C) 1992-1993; ALL RIGHTS RESERVED


  CONTACT ADDRESS

  LENTZ SOFTWARE-DEVELOPMENT    Arjen Lentz @
  Langegracht 7B                AINEX-BBS +31-33-633916
  3811 BT  Amersfoort           FidoNet 2:283/512
  The Netherlands               f512.n283.z2.fidonet.org


  DISCLAIMER

  This information is provided "as is" and comes with no warranties of any
  kind, either expressed or implied. It's intended to be used by programmers
  and developers. In no event shall the author be liable to you or anyone
  else for any damages, including any lost profits, lost savings or other
  incidental or consequential damages arising out of the use or inability
  to use this information.


  LICENCE

  This package may be freely distributed provided the files remain together,
  in their original unmodified form.
  All files, executables and sourcecode remain the copyrighted property of
  Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
  Licence for any use granted, provided this notice & CRC.DOC are included.
  For executable applications, credit should be given in the appropriate
  places in the program and documentation.
  These notices must be retained in any copies of any part of this
  documentation and/or software.

  Any use of, or operation on (including copying/distributing) any of
  the above mentioned files implies full and unconditional acceptance of
  this licence and disclaimer.

=============================================================================*/

#define CRC_TINY
#include "crc.h"


/* ------------------------------------------------------------------------- */
void crc16init (word FAR *crctab, word poly)
{
        register int  i, j;
        register word crc;

        for (i = 0; i <= 15; i++) {
            crc = i;
            for (j = 8; j > 0; j--) {
                if (crc & 1) crc = (crc >> 1) ^ poly;
                else         crc >>= 1;
            }
            crctab[i] = crc;
        }

        for (i = 0; i <= 15; i++) {
            crc = i << 4;
            for (j = 8; j > 0; j--) {
                if (crc & 1) crc = (crc >> 1) ^ poly;
                else         crc >>= 1;
            }
            crctab[16 + i] = crc;
        }
}/*crc16init()*/


/* ------------------------------------------------------------------------- */
word crc16block (word FAR *crctab, word crc, byte FAR *buf, word len)
{
        register byte n;

        while (len--) {
              n = *buf++ ^ (byte) crc;
              crc = crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc >> 8);
        }

        return (crc);
}/*crc16block()*/


/* ------------------------------------------------------------------------- */
word crc16upd (word FAR *crctab, word crc, byte c)
{
        register byte n = c ^ (byte) crc;

        return (crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc >> 8));
}/*crc16upd()*/


/* ------------------------------------------------------------------------- */
void crc16rinit (word FAR *crctab, word poly)
{
        register int  i, j;
        register word crc;

        for (i = 0; i <= 15; i++) {
            crc = i << 8;
            for (j = 8; j > 0; j--) {
                if (crc & 0x8000) crc = (crc << 1) ^ poly;
                else              crc <<= 1;
            }
            crctab[i] = crc;
        }

        for (i = 0; i <= 15; i++) {
            crc = i << 12;
            for (j = 8; j > 0; j--) {
                if (crc & 0x8000) crc = (crc << 1) ^ poly;
                else              crc <<= 1;
            }
            crctab[16 + i] = crc;
        }
}/*crc16rinit()*/


/* ------------------------------------------------------------------------- */
word crc16rblock (word FAR *crctab, word crc, byte FAR *buf, word len)
{
        register byte n;

        while (len--) {
              n = *buf++ ^ (byte) (crc >> 8);
              crc = crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc << 8);
        }

        return (crc);
}/*crc16rblock()*/



/* ------------------------------------------------------------------------- */
word crc16rupd (word FAR *crctab, word crc, byte c)
{
        register byte n = c ^ (byte) (crc >> 8);

        return (crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc << 8));
}/*crc16rupd()*/


/* ------------------------------------------------------------------------- */
void crc32init (dword FAR *crctab, dword poly)
{
        register int   i, j;
        register dword crc;

        for (i = 0; i <= 15; i++) {
            crc = i;
            for (j = 8; j > 0; j--) {
                if (crc & 1) crc = (crc >> 1) ^ poly;
                else         crc >>= 1;
            }
            crctab[i] = crc;
        }

        for (i = 0; i <= 15; i++) {
            crc = i << 4;
            for (j = 8; j > 0; j--) {
                if (crc & 1) crc = (crc >> 1) ^ poly;
                else         crc >>= 1;
            }
            crctab[16 + i] = crc;
        }
}/*crc32init()*/


/* ------------------------------------------------------------------------- */
dword crc32block (dword FAR *crctab, dword crc, byte FAR *buf, word len)
{
        register byte n;

        while (len--) {
              n = *buf++ ^ (byte) crc;
              crc = crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc >> 8);
        }

        return (crc);
}/*crc32block()*/



/* ------------------------------------------------------------------------- */
dword crc32upd (dword FAR *crctab, dword crc, byte c)
{
        register byte n = c ^ (byte) crc;

        return (crctab[n & 0x0f] ^ crctab[16 + ((n >> 4) & 0x0f)] ^ (crc >> 8));
}/*crc32upd()*/


/* end of crc_tiny.c ------------------------------------------------------- */
