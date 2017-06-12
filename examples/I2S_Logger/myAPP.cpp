//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
// application specifific includes
#include "myApp.h"

//============================ Asynchronous Blink =============================================
void blink(uint32_t msec)
{ static uint32_t to=0;
  uint32_t t1 = millis();
  if(t1-to<msec) {yield(); return;}
  digitalWriteFast(13,!digitalReadFast(13)); 
  to=t1;
}

/***************************************************************/
#include "ICS43432.h"

c_ICS43432 ICS43432;

extern "C" void i2sInProcessing(void * s, void * d);

#define N_CHAN 4    // number of channels
#define N_DAT 128   // number of samples per received DMA interrupt 
#define N_BUF 2 * N_CHAN * N_DAT    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];       // buffer for DMA

void logger_init();
void logger_write(uint8_t *data, uint16_t nbuf);
uint32_t logger_save(void);

void c_myApp::setup()
{
  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);
  while(!Serial) blink(100);
  Serial.println("Bird Monitor");

  logger_init();
  logger_save(); // initialize first file
  //
  // initalize and start ICS43432 interface
  ICS43432.init(i2s_rx_buffer, N_BUF);
  ICS43432.start();
  
}

uint32_t i2sProcCount=0;

void c_myApp::loop()
{ 
  static uint16_t appState = 0;
  
  // if we are free running, then blink
  if(appState) {  blink(1000); return;}

  // otherwise runn logger save
  if(logger_save() == (uint32_t)-1)
  { ICS43432.stop(); Serial.println("stopped");pinMode(13,OUTPUT); appState=1; return;}
}

void i2sInProcessing(void * s, void * d)
{
  static uint16_t is_I2S=0;
  if(is_I2S) return;
  is_I2S=1;
  
  i2sProcCount++;
  logger_write((uint8_t *) d, N_CHAN*N_DAT*sizeof(int32_t));
  is_I2S=0;
}

