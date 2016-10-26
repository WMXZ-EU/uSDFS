/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//SD.cpp

/*
 *  attampt to provide SD-type API for uSDFS
 *  function not tested, it only compiles
 */

#include <string.h>
#include "uSD.h"

TCHAR * uSDClass::char2tchar( const char * charString, size_t nn, TCHAR * tcharString)
{ int ii;
  for(ii = 0; ii<nn; ii++) tcharString[ii] = (TCHAR) charString[ii];
  return tcharString;
}

char * uSDClass::tchar2char(TCHAR * tcharString, size_t nn, char * charString)
{ int ii;
  for(ii = 0; ii<nn; ii++) charString[ii] = (char) tcharString[ii];
  return charString;
}

// This needs to be called to set up the connection to the SD card
// before other methods are used.
boolean uSDClass::begin(uint8_t csPin)
{
	FRESULT rc;        	/* Result code */
	TCHAR drive[80];	//(TCHAR *)_T("0:/")
	rc = f_mount (&fatfs, drive , 0);      /* Mount/Unmount a logical drive */
	if(rc == FR_OK) return true;
	return false;
}

// Open the specified file/directory with the supplied mode (e.g. read or
// write, etc). Returns a File object for interacting with the file.
// Note that currently only one file can be open at a time.
boolean uSDClass::open(const char *filename, uint8_t mode)
{
	FRESULT rc;        /* Result code */
	TCHAR wfilename[80];

	unsigned int nn = strlen(filename);

	rc = f_open (&fil, (const TCHAR *) char2tchar(filename, nn, wfilename),(unsigned char) mode);
	if(rc == FR_OK) return true ;
	return false;
}

// Methods to determine if the requested file path exists.
boolean uSDClass::exists(const char *filepath)
{
	FRESULT rc;        /* Result code */
	TCHAR drive[80]; //(TCHAR *)_T("0:/")

	if(rc == FR_OK) return true ;
	return false;
}

// Create the requested directory heirarchy--if intermediate directories
// do not exist they will be created.
boolean uSDClass::mkdir(const char *filepath)
{
	FRESULT rc;        /* Result code */
	TCHAR wfilepath[80]; //(TCHAR *)_T("0:/")

	rc = f_mkdir(wfilepath);
	if(rc == FR_OK) return true ;
	return false;
}

// Delete the file.
boolean uSDClass::remove(const char *filepath)
{
	FRESULT rc;        /* Result code */
	TCHAR wfilepath[80]; //(TCHAR *)_T("0:/")

	rc = f_mkdir(wfilepath);
	if(rc == FR_OK) return true ;
	return false;
}

boolean uSDClass::rmdir(const char *filepath)
{
	FRESULT rc;        /* Result code */
	TCHAR wfilepath[80]; //(TCHAR *)_T("0:/")

	rc = f_mkdir(wfilepath);
	if(rc == FR_OK) return true ;
	return false;
}


size_t uSDClass::write(const uint8_t *buf, size_t size)
{
	FRESULT rc;        /* Result code */
	UINT bw;		// byte written

	rc = f_write(&fil,(const void*) buf, (UINT)size, &bw);			/* Write data to the file */
	if(rc == FR_OK) return bw ;
	return 0;
}

int uSDClass::read(void *buf, uint16_t nbyte)
{
	FRESULT rc;        /* Result code */
	UINT br;		// bytes read

	rc = f_read (&fil, (void*) buf, (UINT) nbyte, &br);			/* Read data from the file */
	if(rc == FR_OK) return br ;
	return 0;
}

