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
// wmxzDSP      // for use of dsp processor
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
#define DO_DSP

/***********************************************************************/
#include "ICS43432.h" // defines also N_BITS

#define F_SAMP 75000
// Note: 
// change either F_CPU or F_SAMP if I2S setup fails to configure
// i.e. there are no DMA interrupts and 'i2sInProcessing' is not running
// typically this happens if the clock generation is unstable
// e.g. F_CPU=168 MHz and F_SAMP = 96000 fails to run
// but F_CPU=168 MHz and F_SMP = 100000 runs fine
//

#define N_CHAN 4    // number of channels

#ifdef DO_AUDIO
  #define AUDIO_SHIFT 8 // for 24 bit to 16 bit conversion (should be larger/equal of 8
  #define ICHAN_LEFT  0
  #define ICHAN_RIGHT 1
#endif

/********************** I2S parameters *******************************/
c_ICS43432 ICS43432;

extern "C" void i2sInProcessing(void * s, void * d);

#define N_DAT (128*(F_SAMP/100)/441)  // number of samples per received DMA interrupt  
#define N_BUF (2 * N_CHAN * N_DAT)    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];         // buffer for DMA

#ifdef DO_DSP
  #define N_FFT 256
  #define N_FILT 4
  
  #if N_DAT>512
    #define N_FFT 1024
    #define N_FILT 1
  #elif N_DAT>256
    #define N_FFT 512
    #define N_FILT 2
  #else
    #define N_FFT 256
    #define N_FILT 4
  #endif
  
  #define MM (N_FFT-N_DAT)  // aka NN-LL
#endif

//
#ifdef DO_AUDIO
/******************************USB-Audio Interface**********************************************************/
  #include <AudioStream.h>
  //
  #include "AudioInterface.h"
  #include "AudioTrigger.h"
  
  static uint8_t audioBuffer[4*N_DAT*4]; // 4 buffers for audio xfer
  c_buff audioStore(audioBuffer,sizeof(audioBuffer));
  
  AudioInterface  interface(&audioStore,F_SAMP);
  AudioOutputUSB  usb;
  AudioConnection patchCord1(interface,0,usb,0);
  //AudioConnection patchCord2(interface,1,usb,1); 
#endif

//======================== Asynchronous Blink ======================================
void blink(uint32_t msec)
{ static uint32_t to=0;
  uint32_t t1 = millis();
  if(t1-to<msec) {yield(); return;}
  digitalWriteFast(13,!digitalReadFast(13)); 
  to=t1;
}

/************************** logger prototypes *************************************/
#ifdef DO_LOGGER
  #include "logger.h"
  header_s header;
#endif

/************************** dsp forward prototypes *************************************/
#ifdef DO_DSP
  int32_t dst[N_CHAN*N_DAT];

  void dsp_init();
  void dsp_exec(int32_t *dst, int32_t *src);
#endif

//======================== I2S processing ======================================

#ifdef DO_AUDIO
  #define AUDIO_NBUF N_DAT
  static int16_t waveform[2*AUDIO_NBUF]; // store for stereo usb-audio data
#endif

uint32_t i2sProcCount=0;

void i2sInProcessing(void * s, void * d)
{
  static uint16_t is_I2S=0;
  if(is_I2S) return;
  is_I2S=1;
  i2sProcCount++;
  
  int32_t *src = (int32_t *) d;
  // for ICS43432 need to shift left to get correct MSB 
  for(int ii=0; ii<N_CHAN*N_DAT;ii++) { src[ii]<<=1;}

  int32_t *out;

#ifdef DO_DSP
  dsp_exec(dst, src);
  out = dst;
#else
  out = src;
#endif

#ifdef DO_LOGGER
  // first logger
  logger_write((uint8_t *) src, N_CHAN*N_DAT*sizeof(int32_t));
#endif

#ifdef DO_AUDIO
  // prepare data for USB-Audio
  // extract data from I2S buffer
  for(int ii=0; ii<AUDIO_NBUF; ii++)
  {  
      waveform[2*ii]  =(int16_t)(out[ICHAN_LEFT +ii*N_CHAN]>>AUDIO_SHIFT);
      waveform[2*ii+1]=(int16_t)(out[ICHAN_RIGHT+ii*N_CHAN]>>AUDIO_SHIFT);
      
//      float arg=2.0f*3.1415926535f*3.0f*(float)ii/(float) AUDIO_NBUF;
//      float amp=1<<3;
//      waveform[2*ii]  =(int16_t)(amp*sinf(arg));
//      waveform[2*ii+1]=(int16_t)(amp*sinf(arg));
  }
  // put data onto audioStore
  audioStore.put((uint8_t *) waveform, 4*AUDIO_NBUF); // 4 = 2x 16-bit channels 
#endif
  is_I2S=0;
}

#ifdef DO_DSP
/**
 *  fft_filt defines NCH (number of channels) and LL (number of samples per block)
 *  NF is number of sub-filters
 *  Let MM = 128 a 128 tab FIR filter (129 sample filter length)
 *  and use of 512 point RFFT
 *  then ACQ block size should be 384 samples (LL = NN-MM)
 *  
 *  alternatively
 *  let AcQ block size to be 217 (128*750/441)
 *  then takeing 1 buffer i.e. LL = 217 samples and MM = 39 for NN = 256 point FFT 
 *  using NF = 4 partitioned filters total FIR length is 160 = 4*40 
 *  
 *  let AcQ block size to be 435 (128*1500/441)
 *  then taking 1 buffer i.e. LL = 435 samples and MM = 77 for NN = 512 point FFT 
 *  using NF = 2 partitioned filters results in total FIR length is 156 = 2*78 
 *  using NF = 4 partitioned filters results in total FIR length is 312 = 4*78 
 */
#include "fft_conv.h"
#include "fft_filt.h"
C_CONV mConv; 

float imp[N_FILT*(MM+1)];
float dsp_buffer[3*N_FFT + 2*N_CHAN*N_FILT*N_FFT + N_CHAN*N_FILT*MM];

float pwr[N_CHAN*N_FFT];

void dsp_init()
{ 
  float fc  =  20.0f/(F_SAMP/2000.0f);
  float dfc =  10.0f/(F_SAMP/2000.0f); 
  calc_FIR_coeffs(imp,N_FILT*(MM+1), fc, ASTOP, BPF, dfc);
  //
  mConv.init(imp, dsp_buffer, N_CHAN, N_FILT, N_DAT, N_FFT, MM);
}

void dsp_exec(int32_t * dst, int32_t *src)
{
    mConv.exec_upos(dst,pwr,src);
}
#endif

/*************************** Arduino compatible Setup ************************************/
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

#ifdef DO_DSP
  dsp_init();
#endif

#ifdef DO_LOGGER
  header.nch = N_CHAN;
  header.fsamp = F_SAMP;
  logger_init(&header);
  logger_save(); // initialize first file
#endif

  //
  // initalize and start ICS43432 interface
  ICS43432.init(F_SAMP, i2s_rx_buffer, N_BUF);
  ICS43432.start();
  
}

/*************************** Arduino loop ************************************/
void c_myApp::loop()
{ 
#ifdef DO_LOGGER
  static uint16_t appState = 0;
  
  // if we are free running, then blink
  if(appState) {  blink(1000); return;}

  // otherwise run logger save
  if(logger_save() == INF)
  { ICS43432.stop(); Serial.println("stopped");pinMode(13,OUTPUT); appState=1; return;}
#else
  static uint32_t t0=0;
  uint32_t t1=millis();
  static uint32_t loopCount=0;
  if (t1-t0>1000) 
  { Serial.printf("%d %d %d %.3f kHz\n\r",
        loopCount,i2sProcCount, N_DAT, ((float)N_DAT*(float)i2sProcCount/1000.0f));
    i2sProcCount=0;
    loopCount=0;
    t0=t1;
  }
  loopCount++;
#endif
}

