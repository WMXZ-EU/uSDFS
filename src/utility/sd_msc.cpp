/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2019 Walter Zimmer.
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
#include "core_pins.h"  // includes calls to kinetis.h or imxrt.h

#include "../diskio.h"
#include "sd_msc.h"
#include "sd_config.h"

#if defined __MK66FX1M0__ || defined __MK64FX512__ || defined __IMXRT1052__ || defined __IMXRT1062__ 
	#define HAVE_MSC USE_MSC	//USE_MSC (0,1) is defined in sd_config
#else
	#define HAVE_MSC 0
#endif

#if HAVE_MSC == 1
	#include <stdlib.h>
	#include <string.h>
	#include "usb_serial.h"
	#include "msc.h"
	#include "MassStorage.h"

// needs msc from  https://github.com/wwatson4506/MSC	
	USBHost myusb;

	int MSC_disk_status() 
	{	
		int stat = 0;
		if(!deviceAvailable()) stat = STA_NODISK; 	// No USB Mass Storage Device Connected
		if(!deviceInitialized()) stat = STA_NOINIT; // USB Mass Storage Device Un-Initialized
		return stat;
	}

	int MSC_disk_initialize() 
	{	myusb.begin();
		return mscInit();
	}

	int MSC_disk_read(BYTE *buff, DWORD sector, UINT count) 
	{	return readSectors((BYTE *)buff, sector, count);
	}

	int MSC_disk_write(const BYTE *buff, DWORD sector, UINT count) 
	{	return writeSectors((BYTE *)buff, sector, count);
	}

	int MSC_ioctl(BYTE cmd, BYTE *buff) {return 0;}
#else
	int MSC_disk_status() {return STA_NOINIT;}
	int MSC_disk_initialize() {return STA_NOINIT;}
	int MSC_disk_read(BYTE *buff, DWORD sector, UINT count) {return STA_NOINIT;}
	int MSC_disk_write(const BYTE *buff, DWORD sector, UINT count) {return STA_NOINIT;}
	int MSC_ioctl(BYTE cmd, BYTE *buff) {return STA_NOINIT;}
#endif