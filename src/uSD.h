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
//SD.h
#ifndef USD_H
#define USD_H

#include "kinetis.h"
#include "ff.h"

#define O_READ 		FA_READ
#define O_RDONLY 	(FA_READ | FA_OPEN_EXISTING)
#define O_WRITE  	FA_WRITE
#define O_CREAT 	FA_CREATE_ALWAYS

#define FILE_READ O_READ
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT)

typedef short boolean;

class uSDClass
{
	private:
	  // These are required for initialisation and use of uSDFS

	  FATFS fatfs;      /* File system object */
	  FIL fil;        /* File object */


	public:
	  // This needs to be called to set up the connection to the SD card
	  // before other methods are used.
	  boolean begin(uint8_t csPin = 10);

	  // Open the specified file/directory with the supplied mode (e.g. read or
	  // write, etc). Returns a File object for interacting with the file.
	  // Note that currently only one file can be open at a time.
	  boolean open(const char *filename, uint8_t mode = FILE_READ);

	  // Methods to determine if the requested file path exists.
	  boolean exists(const char *filepath);

	  // Create the requested directory heirarchy--if intermediate directories
	  // do not exist they will be created.
	  boolean mkdir(const char *filepath);

	  // Delete the file.
	  boolean remove(const char *filepath);

	  boolean rmdir(const char *filepath);

	  size_t write(const uint8_t *buf, size_t size);

	  int read(void *buf, uint16_t nbyte);

	  void close();


	  TCHAR * char2tchar( const char * charString, size_t nn, TCHAR * tcharString);
	  char * tchar2char(  TCHAR * tcharString, size_t nn, char * charString);


};

extern uSDClass SD;

#endif
