/*
 * fft_filt.h
 *
 *  Created on: May 11, 2017
 *      Author: Walter
 */

#ifndef FFT_CONV_H_
#define FFT_CONV_H_

#include "stdint.h"
#include "cmsis.h"
//#include "arm_math.h"
//#include "arm_const_structs.h"

//#define NCH 4       // number of channels
//#define NF 70       // number of Filter partitions
//#define NN 256      // FFT size
//#define MM (NN/2)		// filter length - 1
//#define LL (NN-MM) 	// step size (length of input)

class C_CONV
{
private:
  int nch,nf,ll,nn,mm;
	float *uu; //[NN];
	float *vv; //[NN];
	float *ww; //[NN];
	float *bb; //[NF*NN]; // storage for filter-spectra
	float *zz; //[NCH*NF*NN]; // storage for data-spectra
	//
	float *ov; //[NCH*(NN-LL)]; // storage for data overlap
  int32_t *nj; //[NF];
	//
	arm_rfft_fast_instance_f32 *rfft_instance;

public:
	void init(float *imp, float * buff, int nch, int nf, int ll, int nn, int mm);
	void exec_upos(int32_t *yy, float *pp, int32_t *xx);
};

#endif /* FFT_FILT_H_ */

