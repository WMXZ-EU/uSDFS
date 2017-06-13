//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// in addition to teensy core
// this application needs to be set for 
//  Serial (logger only)
//  Audio  (usb-audio only)
//  Serial + Midi + Audio (logger + usb-audio)
//
// this application needs support from
// uSDFS        // for logger
// wmxzCore     // for I2S, DMA
// wmxzDevices  // for ICS43432
// wmxzAudio    // for USB-monitor
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
// application specifific includes
#include "myApp.h"

#define DO_LOGGER
#define DO_AUDIO

#ifdef DO_AUDIO
/******************************USB-Audio Interface**********************************************************/
#include <AudioStream.h>
//
#include "AudioInterface.h"
#include "AudioTrigger.h"

AudioInterface  interface(48000);
AudioOutputUSB  usb;
AudioConnection patchCord1(interface,0,usb,0);
AudioConnection patchCord2(interface,1,usb,1); // comment for mono
#endif

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
#define N_DAT 139   // number of samples per received DMA interrupt (128*480/441)
#define N_BUF 2 * N_CHAN * N_DAT    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];       // buffer for DMA

#ifdef DO_LOGGER
void logger_init();
void logger_write(uint8_t *data, uint16_t nbuf);
uint32_t logger_save(void);
#endif

void c_myApp::setup()
{
#ifdef DO_AUDIO
  AudioMemory(8);
#endif

  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);
  while(!Serial) blink(100);
  Serial.println("I2S Logger and Monitor");

#ifdef DO_LOGGER
  logger_init();
  logger_save(); // initialize first file
#endif
  //
  // initalize and start ICS43432 interface
  ICS43432.init(i2s_rx_buffer, N_BUF);
  ICS43432.start();
  
}

uint32_t i2sProcCount=0;

void c_myApp::loop()
{ 
#ifdef DO_LOGGER
  static uint16_t appState = 0;
  
  // if we are free running, then blink
  if(appState) {  blink(1000); return;}

  // otherwise run logger save
  if(logger_save() == (uint32_t)-1)
  { ICS43432.stop(); Serial.println("stopped");pinMode(13,OUTPUT); appState=1; return;}
#endif
}


#ifdef DO_AUDIO
#define AUDIO_NBUF N_DAT
#define AUDIO_SHIFT 8 // for 24 bit to 16 bit conversion (should be larger/equal of 8
#define ICHAN_LEFT  0
#define ICHAN_RIGHT 1
static int16_t waveform[2*AUDIO_NBUF];

// circular storage where audio data are stored 
extern c_buff audioStore;
#endif

void i2sInProcessing(void * s, void * d)
{
  static uint16_t is_I2S=0;
  if(is_I2S) return;
  is_I2S=1;
  
  i2sProcCount++;
#ifdef DO_LOGGER
  // first logger
  logger_write((uint8_t *) d, N_CHAN*N_DAT*sizeof(int32_t));
#endif
#ifdef DO_AUDIO
  // prepare data for USB-Audio
  // extract data from I2S buffer
  int32_t *src = (int32_t *) d;
  for(int ii=0; ii<AUDIO_NBUF; ii++)
  {  
      waveform[2*ii]  =(int16_t)(src[ICHAN_LEFT +ii*N_CHAN]>>AUDIO_SHIFT);
      waveform[2*ii+1]=(int16_t)(src[ICHAN_RIGHT+ii*N_CHAN]>>AUDIO_SHIFT);
      
//      float arg=2.0f*3.1415926535f*3.0f*(float)ii/(float) AUDIO_NBUF;
//      float amp=1<<3;
//      waveform[2*ii]  =(int16_t)(amp*sinf(arg));
//      waveform[2*ii+1]=(int16_t)(amp*sinf(arg));
  }
  // put data onto audioStore
  audioStore.put((uint8_t *) waveform, 2*2*AUDIO_NBUF);
#endif  
  is_I2S=0;
}

