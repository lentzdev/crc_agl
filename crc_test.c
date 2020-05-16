/*============================================================ Rev. 15 Jul 1993
  Validation suite and speed trial for table driven CRC-16 & CRC-32
  Should be linked to either CRC.C, CRC.ASM or CRC_TINY.C
  Refer to CRC.DOC for information and documentation.
  -----------------------------------------------------------------------------

	       Information collected and edited by Arjen G. Lentz
	     Sourcecode in C and 80x86 ASM written by Arjen G. Lentz
		  COPYRIGHT (C) 1992-1993; ALL RIGHTS RESERVED


  CONTACT ADDRESS

  LENTZ SOFTWARE-DEVELOPMENT	Arjen Lentz @
  Langegracht 7B		AINEX-BBS +31-33-633916
  3811 BT  Amersfoort		FidoNet 2:283/512
  The Netherlands		f512.n283.z2.fidonet.org


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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crc.h"


enum boolean { false, true };
#ifndef __cplusplus
typedef int boolean;
#endif

enum { CRCNONE, CRC16A, CRC16C, CRC16R, CRC32C };
static int    crctype  = CRCNONE;
static word   poly16, magic16, crc16, check16;
static word  *crc16tab = NULL;
static dword  poly32, magic32, crc32, check32;
static dword *crc32tab = NULL;


/* ------------------------------------------------------------------------- */
#ifdef __MSDOS__
#define CAN_DO_WILDCARDS
#include <stdlib.h>
#include <dos.h>


static char ff_dta[58];

char *ffirst (char *filespec)
{
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	struct SREGS sregs;
#endif
	union  REGS  regs;

	regs.h.ah = 0x1a;
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	sregs.ds  = FP_SEG(ff_dta);
	regs.x.dx = FP_OFF(ff_dta);
	intdosx(&regs,&regs,&sregs);
#else
	regs.x.dx = (word) ff_dta;
	intdos(&regs,&regs);
#endif

	regs.x.cx = 0;
	regs.h.ah = 0x4e;
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	sregs.ds  = FP_SEG(filespec);
	regs.x.dx = FP_OFF(filespec);
	intdosx(&regs,&regs,&sregs);
#else
	regs.x.dx = (word) filespec;
	intdos(&regs,&regs);
#endif
	if (regs.x.cflag)
	   return (NULL);
	return (ff_dta + 0x1e);
}/*ffirst()*/


char *fnext (void)
{
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	struct SREGS sregs;
#endif
	union  REGS  regs;

	regs.h.ah = 0x1a;
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	sregs.ds  = FP_SEG(ff_dta);
	regs.x.dx = FP_OFF(ff_dta);
	intdosx(&regs,&regs,&sregs);
#else
	regs.x.dx = (word) ff_dta;
	intdos(&regs,&regs);
#endif

	regs.h.ah = 0x4f;
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
	intdosx(&regs,&regs,&sregs);
#else
	intdos(&regs,&regs);
#endif
	if (regs.x.cflag)
	   return (NULL);
	return (ff_dta + 0x1e);
}/*fnext()*/


#endif/*__MSDOS__*/


/* ------------------------------------------------------------------------- */
#if __TOS__
#define CAN_DO_WILDCARDS
#include <stdlib.h>
#include <tos.h>


static char fname[20];
static DTA tdta;

char *ffirst (char *name)
{
	DTA *hisdta = Fgetdta();
	char *q, *p;
	char *temp;

	Fsetdta(&tdta);

	if (Fsfirst(name,7) != -33L) {
	    p = fname;
	    for (q = tdta.d_fname; *q; ) *p++ = *q++;
	    *p = '\0';
	    temp = fname;
	}
	else
	    temp = NULL;

	Fsetdta(hisdta);

	return (temp);
}/*ffirst()*/


char *fnext (void)
{
	DTA *hisdta = Fgetdta();
	char *q, *p;
	char *temp;

	Fsetdta(&tdta);

	if (Fsnext() == 0L) {
	   p = fname;
	   for (q = tdta.d_fname; *q; ) *p++ = *q++;
	   *p='\0';
	   temp = fname;
	}
	else
	   temp = NULL;

	Fsetdta(hisdta);

	return (temp);
}/*fnext()*/


#endif/*__TOS__*/


/* ------------------------------------------------------------------------- */
#ifdef CAN_DO_WILDCARDS
void splitpath (char *filepath, char *path, char *file)
{
	register char *p, *q;

	for (p = filepath; *p; p++);
	while (p != filepath && *p != ':' && *p != '\\' && *p != '/') p--;
	if (*p == ':' || *p == '\\' || *p == '/') p++;
	q = filepath;
	while (q != p) *path++ = *q++;
	*path = '\0';
	strcpy(file,p);
}/*splitpath()*/
#endif


/* ------------------------------------------------------------------------- */
void do_magic (void)
{
	char	buf1[10], buf2[10];
	boolean res = true;

	switch (crctype) {
	       case CRC16A: magic16 = crc16block(crc16tab,CRC16AINIT,(byte *) "\0\0",2);
			    if (magic16 != 0U) {
			       sprintf(buf1,"%04X",magic16);
			       sprintf(buf2,"%04X",CRC16ATEST);
			       res = false;
			    }
			    break;
	       case CRC16C: magic16 = crc16block(crc16tab,CRC16INIT,(byte *) "\0\0",2);
			    if (poly16 == CRC16POLY && magic16 != CRC16TEST) {
			       sprintf(buf1,"%04X",magic16);
			       sprintf(buf2,"%04X",CRC16TEST);
			       res = false;
			    }
			    break;
	       case CRC16R: magic16 = crc16rblock(crc16tab,CRC16RINIT,(byte *) "\0\0",2);
			    if (magic16 != 0U) {
			       sprintf(buf1,"%04X",magic16);
			       sprintf(buf2,"%04X",CRC16RTEST);
			       res = false;
			    }
			    break;
	       case CRC32C: magic32 = crc32block(crc32tab,CRC32INIT,(byte *) "\0\0\0\0",4);
			    if (poly32 == CRC32POLY && magic32 != CRC32TEST) {
			       sprintf(buf1,"%08lX",magic32);
			       sprintf(buf2,"%08lX",CRC32TEST);
			       res = false;
			    }
			    break;
	}

	if (!res) {
	   printf("Magic check number is incorrect: %s instead of %s\n",buf1,buf2);
	   printf("This could indicate a compiler code problem...\n");
	   printf("If there are a lot of Fs, AND operations may be required after bitshifting.\n");
	   printf("Otherwise, it could be a fault somewhere in the CRC table generator\n");
	}
}/*do_magic()*/


/* ------------------------------------------------------------------------- */
void do_test (void)
{
	boolean res = true;

	switch (crctype) {
	       case CRC16A:
	       case CRC16C: check16 = crc16upd(crc16tab,check16,crc16);
			    check16 = crc16upd(crc16tab,check16,crc16 >> 8);
			    if (check16 != magic16) res = false;
			    break;
	       case CRC16R: check16 = crc16rupd(crc16tab,check16,crc16 >> 8);
			    check16 = crc16rupd(crc16tab,check16,crc16);
			    if (check16 != magic16) res = false;
			    break;
	       case CRC32C: check32 = crc32upd(crc32tab,check32,(int) crc32);
			    check32 = crc32upd(crc32tab,check32,(int) (crc32 >> 8));
			    check32 = crc32upd(crc32tab,check32,(int) (crc32 >> 16));
			    check32 = crc32upd(crc32tab,check32,(int) (crc32 >> 24));
			    if (check32 != CRC32TEST) res = false;
			    break;
	}

	if (!res) {
	   printf("Result is incorrect!  This could indicate a compiler code problem...\n");
	   printf("If there are a lot of Fs, AND operations may be required after bitshifting.\n");
	}
}/*do_test()*/


/* ------------------------------------------------------------------------- */
void do_table (void)
{
	register int i;

	switch (crctype) {
	       case CRC16A: printf("/* CRC-16 with init 0, send as is, test 0 */\n");
			    printf("/* Polynomial %04X (%s), %u-entry table */\n",
				   poly16,poly16 == CRC16APOLY ? "as in ARC,LHA" : "Userdef",CRC_TABSIZE);
			    printf("word crc16atab[%u] = {\n",CRC_TABSIZE);
			    break;
	       case CRC16C: printf("/* CRC-16 - init 1s, send ones complement, test magic %04X */\n",magic16);
			    printf("/* Polynomial %04X (%s), %u-entry table */\n",
				   poly16,poly16 == CRC16POLY ? "CCITT; as in X.25,HDLC,Hydra" : "Userdef",CRC_TABSIZE);
			    printf("word crc16tab[%u] = {\n",CRC_TABSIZE);
			    break;
	       case CRC16R: printf("/* Upside-down CRC-16, init 0s, send as is, test 0 */\n");
			    printf("/* Polynomial %04X (%s), %u-entry table */\n",
				   poly16,poly16 == CRC16RPOLY ? "X/Y/Zmodem" : "Userdef",CRC_TABSIZE);
			    printf("word crc16rtab[%u] = {\n",CRC_TABSIZE);
			    break;
	       case CRC32C: printf("/* CRC-32 - init 1s, send ones complement, test magic %08lX */\n",magic32);
			    printf("/* Polynomial %08lX (%s), %u-entry table */\n",
				   poly32,poly32 == CRC32POLY ? "CCITT; as in Zmodem,Hydra" : "Userdef",CRC_TABSIZE);
			    printf("dword crc32tab[%u] = {\n",CRC_TABSIZE);
			    break;
	}

	for (i = 0; i < CRC_TABSIZE; i++) {
	    if (crctype == CRC32C)
	       printf("0x%08lXL",crc32tab[i]);
	    else {
	       if (!(i % 8)) printf("        ");
	       printf("0x%04X",crc16tab[i]);
	    }
	    if (i < (CRC_TABSIZE - 1)) printf(",");
	    if (!((i + 1) % 8)) printf("\n");
	    else		printf(" ");
	}
	printf("};\n");
}/*do_table()*/


/* ------------------------------------------------------------------------- */
void do_perf (void)
{
	byte buf[1024];
	word i;
	long start, stop, t;

	for (i = 0; i < 1024; i++) buf[i] = (byte) i;
	printf("Processing 10,240 1024-byte blocks (10MB)...\n");

	switch (crctype) {
	       case CRC16A: crc16 = CRC16AINIT;
			    time(&start);
			    for (i = 0; i < 10240; i++)
				crc16 = crc16block(crc16tab,crc16,buf,1024);
			    time(&stop);
			    check16 = crc16;
			    crc16 = CRC16APOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16C: crc16 = CRC16INIT;
			    time(&start);
			    for (i = 0; i < 10240; i++)
				crc16 = crc16block(crc16tab,crc16,buf,1024);
			    time(&stop);
			    check16 = crc16;
			    crc16 = CRC16POST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16R: crc16 = CRC16RINIT;
			    time(&start);
			    for (i = 0; i < 10240; i++)
				crc16 = crc16rblock(crc16tab,crc16,buf,1024);
			    time(&stop);
			    check16 = crc16;
			    crc16 = CRC16RPOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC32C: crc32 = CRC32INIT;
			    time(&start);
			    for (i = 0; i < 10240; i++)
				crc32 = crc32block(crc32tab,crc32,buf,1024);
			    time(&stop);
			    check32 = crc32;
			    crc32 = CRC32POST(crc32);
			    sprintf((char *) buf,"%08lX",crc32);
			    break;
	}

	t = stop - start;
	if (t < 0)
	   t += 24L * 3600L;

	printf("CRC of 10MB test input is %s (for this CRC type/polynomial).\n", buf);
	do_test();
	printf("Processing time : %u secs\n", t);
	printf("Bytes per second: %ld\n", (long) ((10240L * 1024L) / t));
}/*do_perf()*/


/* ------------------------------------------------------------------------- */
void do_string (char *s)
{
	char buf[10];

	switch (crctype) {
	       case CRC16A: crc16 = crc16block(crc16tab,CRC16AINIT,(byte *) s,(int) strlen(s));
			    check16 = crc16;
			    crc16 = CRC16APOST(crc16);
			    sprintf(buf,"%04X",crc16);
			    break;
	       case CRC16C: crc16 = crc16block(crc16tab,CRC16INIT,(byte *) s,(int) strlen(s));
			    check16 = crc16;
			    crc16 = CRC16POST(crc16);
			    sprintf(buf,"%04X",crc16);
			    break;
	       case CRC16R: crc16 = crc16rblock(crc16tab,CRC16RINIT,(byte *) s,(int) strlen(s));
			    check16 = crc16;
			    crc16 = CRC16RPOST(crc16);
			    sprintf(buf,"%04X",crc16);
			    break;
	       case CRC32C: crc32 = crc32block(crc32tab,CRC32INIT,(byte *) s,(int) strlen(s));
			    check32 = crc32;
			    crc32 = CRC32POST(crc32);
			    sprintf(buf,"%08lX",crc32);
			    break;
	}

	printf("%s \"%s\"\n", buf, s);
	do_test();
}/*do_string()*/


/* ------------------------------------------------------------------------- */
void do_stdio (void)
{
	byte  buf[1024];
	int   i;

	printf("Reading from stdin until EOF....\n");

	switch (crctype) {
	       case CRC16A: crc16 = CRC16AINIT;
			    while ((i = (int) fread(buf,1,1024,stdin)) != 0)
				  crc16 = crc16block(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16APOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16C: crc16 = CRC16INIT;
			    while ((i = (int) fread(buf,1,1024,stdin)) != 0)
				  crc16 = crc16block(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16POST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16R: crc16 = CRC16RINIT;
			    while ((i = (int) fread(buf,1,1024,stdin)) != 0)
				  crc16 = crc16rblock(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16RPOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC32C: crc32 = CRC32INIT;
			    while ((i = (int) fread(buf,1,1024,stdin)) != 0)
				  crc32 = crc32block(crc32tab,crc32,buf,i);
			    check32 = crc32;
			    crc32 = CRC32POST(crc32);
			    sprintf((char *) buf,"%08lX",crc32);
			    break;
	}

	printf("CRC = %s\n", buf);
	do_test();
}/*do_stdio()*/


/* ------------------------------------------------------------------------- */
boolean do_file (char *filename)
{
	FILE *fp;
	byte  buf[1024];
	int   i;

	if ((fp = fopen(filename,"rb")) == NULL) {
	   printf("Can't open %s\n",filename);
	   return (false);
	}

	switch (crctype) {
	       case CRC16A: crc16 = CRC16AINIT;
			    while ((i = (int) fread(buf,1,1024,fp)) != 0)
				  crc16 = crc16block(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16APOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16C: crc16 = CRC16INIT;
			    while ((i = (int) fread(buf,1,1024,fp)) != 0)
				  crc16 = crc16block(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16POST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC16R: crc16 = CRC16RINIT;
			    while ((i = (int) fread(buf,1,1024,fp)) != 0)
				  crc16 = crc16rblock(crc16tab,crc16,buf,i);
			    check16 = crc16;
			    crc16 = CRC16RPOST(crc16);
			    sprintf((char *) buf,"%04X",crc16);
			    break;
	       case CRC32C: crc32 = CRC32INIT;
			    while ((i = (int) fread(buf,1,1024,fp)) != 0)
				  crc32 = crc32block(crc32tab,crc32,buf,i);
			    check32 = crc32;
			    crc32 = CRC32POST(crc32);
			    sprintf((char *) buf,"%08lX",crc32);
			    break;
	}

	fclose(fp);

	printf("%s %s\n", buf, filename);
	do_test();

	return (true);
}/*do_file()*/


/* ------------------------------------------------------------------------- */
void main (int argc, char *argv[])
{
	int ac;

	printf("CRC_TEST; A validation program for CRC-16 and CRC-32 calculations...\n");
	printf("Source code COPYRIGHT (C) 1992-1993 by Arjen G. Lentz; ALL RIGHTS RESERVED\n\n");

	if (argc < 3) goto usage;

	if (!strnicmp(argv[1],"-a",2)) {
	   crctype = CRC16A;
	   if (sscanf(&argv[1][2],"%04x",&poly16) != 1)
	      poly16 = CRC16APOLY;
	   if ((crc16tab = (word *) malloc(CRC_TABSIZE * sizeof (word))) != NULL)
	      crc16init(crc16tab,poly16);
	}
	else if (!strnicmp(argv[1],"-c",2)) {
	   crctype = CRC16C;
	   if (sscanf(&argv[1][2],"%04x",&poly16) != 1)
	      poly16 = CRC16POLY;
	   if ((crc16tab = (word *) malloc(CRC_TABSIZE * sizeof (word))) != NULL)
	      crc16init(crc16tab,poly16);
	}
	else if (!strnicmp(argv[1],"-r",2)) {
	   crctype = CRC16R;
	   if (sscanf(&argv[1][2],"%04x",&poly16) != 1)
	      poly16 = CRC16RPOLY;
	   if ((crc16tab = (word *) malloc(CRC_TABSIZE * sizeof (word))) != NULL)
	      crc16rinit(crc16tab,poly16);
	}
	else if (!strnicmp(argv[1],"-d",2)) {
	   crctype = CRC32C;
	   if (sscanf(&argv[1][2],"%08lX",&poly32) != 1)
	      poly32 = CRC32POLY;
	   if ((crc32tab = (dword *) malloc(CRC_TABSIZE * sizeof (dword))) != NULL)
	      crc32init(crc32tab,poly32);
	}
	else goto usage;

	if (crc16tab == NULL && crc32tab == NULL) {
	   printf("Could not allocate CRC table\n");
	   return;
	}

	do_magic();

	for (ac = 2; ac < argc; ac++) {
	    if (!strnicmp(argv[ac],"-o",2))
	       do_table();
	    else if (!strnicmp(argv[ac],"-s",2))
	       do_string(&argv[ac][2]);
	    else if (!strnicmp(argv[ac],"-p",2))
	       do_perf();
	    else if (!strnicmp(argv[ac],"-t",2)) {
	       printf("Standard test suite:\n");
	       do_string("");
	       do_string("a");
	       do_string("abc");
	       do_string("message crc");
	       do_string("abcdefghijklmnopqrstuvwxyz");
	       do_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	       do_string("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
	       do_string("hi");
	    }
	    else if (!strnicmp(argv[ac],"-i",2))
	       do_stdio();
	    else {
	       boolean found = false;
#ifdef CAN_DO_WILDCARDS
	       char pathname[128], path[100], name[20];
	       char *p;

	       splitpath(argv[ac],path,name);
	       for (p = ffirst(argv[ac]); p; p = fnext()) {
		   strcpy(pathname,path);
		   strcat(pathname,p);
		   if (do_file(pathname)) found = true;
	       }
#else
	       if (do_file(argv[ac])) found = true;
#endif
	       if (!found)
		  printf("No files found matching '%s'\n",argv[ac]);
	    }
	}

	if (crc16tab != NULL) free(crc16tab);
	if (crc32tab != NULL) free(crc32tab);
	return;

usage:	printf("Usage: CRC_TEST <type> <cmd> ...\n");
	printf("type  -a          CRC-16 with init 0 (as in ARC,LHA)\n");
	printf("      -aXXXX      CRC-16 with init 0 using polynomial hex XXXX\n");
	printf("      -c          CRC-16 CCITT polynomial (as in X.25,HDLC,Hydra)\n");
	printf("      -cXXXX      CRC-16 using polynomial hex XXXX\n");
	printf("      -r          Upside-down CRC-16 (as in X/Y/Zmodem)\n");
	printf("      -rXXXX      Upside-down CRC-16 using polynomial hex XXXX\n");
	printf("      -d          CRC-32 CCITT polynomial (as in Zmodem,Hydra)\n");
	printf("      -dXXXXXXXX  CRC-32 using polynomial hex XXXXXXXX\n");
	printf("\n");
	printf("cmd   -o          Output the calculated CRC table as a C array, + magic number\n");
	printf("      -s<text>    Print CRC for specified text string\n");
	printf("      -p          Performance test on 10Mbytes\n");
	printf("      -t          Standard suite of test data\n");
	printf("      <file>      Print CRC for specified file(s)\n");
	printf("      -i          Ditto, but use stdin for input\n");
	printf("\n");
	printf("Note: All calculations are checked (not all code/compiler errors detected!)\n");
	printf("      For standard polynomials and 0 inits, pre-compiled check results are used\n");
}/*main()*/


/* end of crc_test.c ------------------------------------------------------- */
