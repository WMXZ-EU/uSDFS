//Copyright 2017 by Walter Zimmer
// Version 22-06-17
//
#ifndef LOGGER_H
#define LOGGER_H

/****************** Custom ************************************************/
#define MXFN 5 					// maximal number of files 
#define MAX_BLOCK_COUNT 1000	// number of BUFFSIZE writes to file (defines file size)

#define FMT "X_%05u.dat"		// defines filename


/*********************** system ******************************************/
#define INF ((uint32_t) (-1))

typedef struct
{
  uint32_t rtc;
  uint32_t t0;
  uint32_t nch;
  uint32_t fsamp;
} header_s;

// function prototypes
void logger_init(header_s * header);
void logger_write(uint8_t *data, uint16_t nbuf);
uint32_t logger_save(void);

#endif
